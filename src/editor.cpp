#include "editor.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>

EditorConfig::EditorConfig() {
    // Заглушка: Здесь можно задать начальные значения для редактора.
    setStatusMessage("Press Ctrl-Q to quit");
}

void EditorConfig::setStatusMessage(const std::string& message) {
    statusMessage = message;
    statusMessageTime = std::time(nullptr);
}

void EditorConfig::refreshScreen() {
    // очистка экрана
    std::cout << "\x1b[2J";
    std::cout << "\x1b[H";
    for (int y = 0; y < rows.size(); ++y) {
        std::cout << rows[y].content << "\n";
    }
    std::cout << "\x1b[" << cursorY + 1 << ";" << cursorX + 1 << "H";
    std::cout << statusMessage << std::flush;
}

void EditorConfig::processKeypress() {
    char c;
    read(STDIN_FILENO, &c, 1);

    switch (c) {
        case static_cast<char>(KeyAction::Enter):
            insertRow("");
            cursorY++;
            cursorX = 0;
            break;
        case static_cast<char>(KeyAction::CtrlQ):
            std::cout << "\x1b[2J";
            std::cout << "\x1b[H";
            exit(0);
            break;
        case static_cast<char>(KeyAction::Backspace):
            if (cursorX > 0) {
                rows[cursorY].content.erase(cursorX - 1, 1);
                cursorX--;
            }
            break;
        default:
            if (isprint(c)) {
                rows[cursorY].content.insert(cursorX, 1, c);
                cursorX++;
            }
    }
    dirty = true;
}

void EditorConfig::insertRow(const std::string& line) {
    rows.emplace_back(rows.size(), line);
}

void enableRawMode() {
    termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode() {
    termios original;
    tcgetattr(STDIN_FILENO, &original);
    original.c_lflag |= (ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

int main() {
    enableRawMode();
    EditorConfig editor;
    editor.insertRow("");

    while (true) {
        editor.refreshScreen();
        editor.processKeypress();
    }

    disableRawMode();
    return 0;
}

