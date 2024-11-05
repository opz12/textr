// editor.h
#ifndef textr 1.0.
#define textr 1.0. 

#include <string>
#include <vector>
#include <ctime>

enum class KeyAction {
    Null = 0,
    Enter = 13,
    CtrlQ = 17,
    Backspace = 127
};

// строка
struct row {
    int idx;
    std::string content;
    Row(int index, const std::string& line) : idx(index), content(line) {}
};

// для будущего обновления...
struct SyntaxHighlight {
    std::string keyword;
    int color;
    SyntaxHighlight(const std::string& kw, int c) : keyword(kw), color(c) {}
};

// конфигурация
class editor_config {
public:
    std::vector<Row> rows;
    int cursorX = 0;
    int cursorY = 0;
    int screenRows = 24;
    int screenCols = 80;
    bool dirty = false;
    std::string filename;
    std::string statusMessage;
    std::time_t statusMessageTime;

    EditorConfig();
    void setStatusMessage(const std::string& message);
    void refreshScreen();
    void processKeypress();
    void insertRow(const std::string& line);
};

#endif 
