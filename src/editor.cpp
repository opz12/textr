#include "editor.h"
#include <sstream>

editor_config::editor_config() {
    message_status("Press Q to quit | S to save | F to find | U to undo | R to redo");
}

void editor_config::open_file(const std::string& filename) {
    this->filename = filename;
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            rows.push_back(Row(line));
        }
        file.close();
        dirty = false;
    }
}

void editor_config::save_file() {
    if (filename.empty()) return;
    std::ofstream file(filename);
    if (file.is_open()) {
        for (auto& row : rows) {
            file << row.content << "\n";
        }
        file.close();
        dirty = false;
        message_status("File saved successfully");
    } else {
        message_status("Error saving file");
    }
}

void editor_config::message_status(const std::string& message) {
    statusMessage = message;
    statusMessageTime = std::time(nullptr);
}

void editor_config::screen_refresh() {
    scroll();
    std::cout << "\033[2J\033[H"; // Clears screen and moves cursor to top left
    drawRows();
    drawStatusBar();
    int cx = cursorX - colOffset + 1;
    int cy = cursorY - rowOffset + 1;
    std::cout << "\033[" << cy << ";" << cx << "H" << std::flush;
}

void editor_config::scroll() {
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

void editor_config::drawRows() {
    for (int y = 0; y < screenRows; y++) {
        int fileRow = y + rowOffset;
        if (fileRow < rows.size()) {
            Row &row = rows[fileRow];
            apply_syntax_highlighting(row);
            std::string line = row.content;
            if (line.length() > colOffset) {
                line = line.substr(colOffset, screenCols);
            }
            std::cout << line;
        }
        std::cout << "\033[K\n"; // Clear to end of line
    }
}

void editor_config::drawStatusBar() {
    std::cout << "\033[7m"; // Inverted colors
    std::string status = filename + (dirty ? " (modified)" : "");
    std::string info = "Cursor: (" + std::to_string(cursorY + 1) + "," + std::to_string(cursorX + 1) + ")";
    status.resize(screenCols - info.size(), ' ');
    std::cout << status << info << "\033[m\n";
}

void editor_config::move_cursor(char key) {
    switch (key) {
        case 'h': // Left
            if (cursorX > 0) cursorX--;
            break;
        case 'l': // Right
            if (cursorY < rows.size() && cursorX < rows[cursorY].content.length()) cursorX++;
            break;
        case 'k': // Up
            if (cursorY > 0) cursorY--;
            break;
        case 'j': // Down
            if (cursorY < rows.size() - 1) cursorY++;
            break;
        case '0': // Home
            cursorX = 0;
            break;
        case '$': // End
            cursorX = rows[cursorY].content.size();
            break;
        default:
            break;
    }
}

void editor_config::char_insert(char c) {
    if (cursorY >= rows.size()) rows.emplace_back("");
    undoStack.push(rows);  // Save the current state for undo
    rows[cursorY].content.insert(cursorX, 1, c);
    cursorX++;
    dirty = true;
}

void editor_config::char_delete() {
    if (cursorY < rows.size() && cursorX > 0) {
        undoStack.push(rows); // Save the current state for undo
        rows[cursorY].content.erase(cursorX - 1, 1);
        cursorX--;
        dirty = true;
    }
}

void editor_config::undo() {
    if (!undoStack.empty()) {
        redoStack.push(rows); // Save the current state for redo
        rows = undoStack.top();
        undoStack.pop();
        dirty = true;
    }
}

void editor_config::redo() {
    if (!redoStack.empty()) {
        undoStack.push(rows); // Save the current state for undo
        rows = redoStack.top();
        redoStack.pop();
        dirty = true;
    }
}

void editor_config::search_prompt() {
    std::string query;
    std::cout << "Search: ";
    std::cin >> query;
    search(query);
}

void editor_config::search(const std::string& query) {
    int current = cursorY;
    for (int i = 0; i < rows.size(); i++) {
        int lineIndex = (current + i) % rows.size();
        int pos = rows[lineIndex].content.find(query);
        if (pos != std::string::npos) {
            cursorY = lineIndex;
            cursorX = pos;
            rowOffset = cursorY;
            message_status("Match found");
            return;
        }
    }
    message_status("No matches found");
}

void editor_config::key_process() {
    char c;
    std::cin >> c;

    switch (c) {
        case 'Q':
            std::cout << "\033[2J\033[H";
            exit(0);
        case 'S':
            save_file();
            break;
        case 'F':
            search_prompt();
            break;
        case 'U':
            undo();
            break;
        case 'R':
            redo();
            break;
        case 127: // Backspace
            char_delete();
            break;
        case '\n': // Enter
            rows.insert(rows.begin() + cursorY + 1, Row(""));
            cursorY++;
            cursorX = 0;
            dirty = true;
            break;
        default:
            if (std::isprint(c)) {
                char_insert(c);
            }
            break;
    }
}

void editor_config::apply_syntax_highlighting(Row &row) {
    static const std::unordered_set<std::string> keywords = {"int", "float", "return", "if", "else"};
    std::string &content = row.content;
    std::string highlighted;
    std::istringstream iss(content);
    std::string word;

    while (iss >> word) {
        if (keywords.count(word)) {
            highlighted += "\033[1;32m" + word + "\033[0m ";  // Green color for keywords
        } else {
            highlighted += word + " ";
        }
    }
    content = highlighted;
}

int main(int argc, char* argv[]) {
    editor_config editor;

    if (argc >= 2) {
        editor.open_file(argv[1]);
    } else {
        editor.rows.emplace_back("");
    }

    while (true) {
        editor.screen_refresh();
        editor.key_process();
    }

    return 0;
}


    disableRawMode();
    return 0;
}

