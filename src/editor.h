#ifndef EDITOR_H
#define EDITOR_H

#include <vector>
#include <string>
#include <stack>
#include <iostream>
#include <fstream>
#include <ctime>
#include <unordered_set>

struct Row {
    std::string content;
    Row(const std::string &str) : content(str) {}
};

class editor_config {
public:
    std::vector<Row> rows;
    std::stack<std::vector<Row>> undoStack; // Undo stack
    std::stack<std::vector<Row>> redoStack; // Redo stack
    
    std::string filename;
    std::string statusMessage;
    time_t statusMessageTime;
    int cursorX = 0, cursorY = 0;
    int screenRows = 24, screenCols = 80;
    int rowOffset = 0, colOffset = 0;
    bool dirty = false;
    
    editor_config();
    void open_file(const std::string& filename);
    void save_file();
    void message_status(const std::string& message);
    void screen_refresh();
    void scroll();
    void drawRows();
    void drawStatusBar();
    void move_cursor(char key);
    void char_insert(char c);
    void char_delete();
    void undo();
    void redo();
    void search_prompt();
    void search(const std::string& query);
    void key_process();
    void apply_syntax_highlighting(Row &row);
};

#endif

