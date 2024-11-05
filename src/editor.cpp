#include "editor.h"
#include <iostream>
#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <ctime>

EditorConfig::EditorConfig() {
    setStatusMessage("Press Ctrl-Q to quit | Ctrl-S to save | Ctrl-F to find");
}

// Открытие файла
void EditorConfig::openFile(const std::string& filename) {
    this->filename = filename;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            rows.push_back(Row(line));
        }
        file.close();
    }
}

// Сохранение файла
void EditorConfig::saveFile() {
    if (filename.empty()) return;
    std::ofstream file(filename);
    if (file.is_open()) {
        for (auto& row : rows) {
            file << row.content << "\n";
        }
        file.close();
        dirty = false;
        setStatusMessage("File saved successfully");
    } else {
        setStatusMessage("Error saving file");
    }
}

// Установка сообщения состояния
void EditorConfig::setStatusMessage(const std::string& message) {
    statusMessage = message;
    statusMessageTime = std::time(nullptr);
}

// Перерисовка экрана с учетом прокрутки
void EditorConfig::refreshScreen() {
    scroll();
    std::cout << "\x1b[2J"; // Очистка экрана
    std::cout << "\x1b[H";  // Перемещение в начало

    drawRows();
    drawStatusBar();

    int cx = cursorX - colOffset + 1;
    int cy = cursorY - rowOffset + 1;
    std::cout << "\x1b[" << cy << ";" << cx << "H";
    std::cout << std::flush;
}

// Прокрутка
void EditorConfig::scroll() {
    if (cursorY < rowOffset) {
        rowOffset = cursorY;
    }
    if (cursorY >= rowOffset + screenRows) {
        rowOffset = cursorY - screenRows + 1;
    }
    if (cursorX < colOffset) {
        colOffset = cursorX;
    }
    if (cursorX >= colOffset + screenCols) {
        colOffset = cursorX - screenCols + 1;
    }
}

// Отрисовка строк текста
void EditorConfig::drawRows() {
    for (int y = 0; y < screenRows; y++) {
        int fileRow = y + rowOffset;
        if (fileRow < rows.size()) {
            std::string line = rows[fileRow].content;
            if (line.length() > colOffset) {
                line = line.substr(colOffset, screenCols);
            }
            std::cout << line;
        }
        std::cout << "\x1b[K\n"; // Очистка до конца строки
    }
}

// Отрисовка строки состояния
void EditorConfig::drawStatusBar() {
    std::cout << "\x1b[7m"; // Инвертированные цвета для строки состояния
    std::string status = filename + (dirty ? " (modified)" : "");
    std::string info = "Cursor: (" + std::to_string(cursorY + 1) + "," + std::to_string(cursorX + 1) + ")";
    status.resize(screenCols - info.size(), ' ');
    std::cout << status << info << "\x1b[m\n";
}

// Перемещение курсора
void EditorConfig::moveCursor(KeyAction key) {
    switch (key) {
        case KeyAction::ArrowLeft:
            if (cursorX > 0) cursorX--;
            break;
        case KeyAction::ArrowRight:
            if (cursorY < rows.size() && cursorX < rows[cursorY].content.length()) cursorX++;
            break;
        case KeyAction::ArrowUp:
            if (cursorY > 0) cursorY--;
            break;
        case KeyAction::ArrowDown:
            if (cursorY < rows.size() - 1) cursorY++;
            break;
        case KeyAction::Home:
            cursorX = 0;
            break;
        case KeyAction::End:
            cursorX = rows[cursorY].content.size();
            break;
        default:
            break;
    }
}

// Вставка символа
void EditorConfig::insertChar(char c) {
    if (cursorY >= rows.size()) rows.emplace_back("");
    rows[cursorY].content.insert(cursorX, 1, c);
    cursorX++;
    dirty = true;
}

// Удаление символа
void EditorConfig::deleteChar() {
    if (cursorY < rows.size() && cursorX > 0) {
        rows[cursorY].content.erase(cursorX - 1, 1);
        cursorX--;
        dirty = true;
    }
}

// Поиск текста
void EditorConfig::search(const std::string& query) {
    int current = cursorY;
    for (int i = 0; i < rows.size(); i++) {
        int lineIndex = (current + i) % rows.size();
        int pos = rows[lineIndex].content.find(query);
        if (pos != std::string::npos) {
            cursorY = lineIndex;
            cursorX = pos;
            rowOffset = cursorY;
            setStatusMessage("Match found");
            return;
        }
    }
    setStatusMessage("No matches found");
}

// Обработка ввода
void EditorConfig::processKeypress() {
    char c;
    read(STDIN_FILENO, &c, 1);

    switch (c) {
        case 17: // Ctrl-Q
            std::cout << "\x1b[2J\x1b[H";
            exit(0);
            break;
        case 19: // Ctrl-S
            saveFile();
            break;
        case 6: // Ctrl-F
            search("sample"); // Строка для поиска — можно сделать запрашиваемой у пользователя
            break;
        case 127: // Backspace
            deleteChar();
            break;
        case 13: // Enter
            if (cursorY < rows.size()) {
                rows.insert(rows.begin() + cursorY + 1, Row(""));
                cursorY++;
                cursorX = 0;
                dirty = true;
            }
            break;
        default:
            if (isprint(c)) {
                insertChar(c);
            }
            break;
    }
}

// Включение "сырого" режима терминала
void enableRawMode() {
    termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Выключение "сырого" режима
void disableRawMode() {
    termios original;
    tcgetattr(STDIN_FILENO, &original);
    original.c_lflag |= (ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

int main(int argc, char* argv[]) {
    enableRawMode();
    EditorConfig editor;

    if (argc >= 2) {
        editor.openFile(argv[1]);
    } else {
        editor.rows.emplace_back("");
    }

    while (true) {
        editor.refreshScreen();
        editor.processKeypress();
    }

    disableRawMode();
    return 0;
}

