#ifndef textr 1.0.
#define textr 1.0. 

#include <string>
#include <vector>
#include <ctime>


// Коды для клавиш
enum class KeyAction {
    Null = 0,
    Enter = 13,
    CtrlQ = 17,
    Backspace = 127,
    ArrowLeft = 1000,
    ArrowRight,
    ArrowUp,
    ArrowDown,
    Save,
    Home,
    End,
    PageUp,
    PageDown,
    Find
};

// Структура строки текста
struct Row {
    std::string content;
    Row(const std::string& line) : content(line) {}
};

// Основная конфигурация редактора
class EditorConfig {
private:
    std::string statusMessage;
    std::time_t statusMessageTime;
    void drawRows();
    void drawStatusBar();
    void scroll();
    int findTextInRow(const std::string& text, int startRow);

public:
    int cursorX = 0, cursorY = 0;       // Позиция курсора
    int rowOffset = 0, colOffset = 0;   // Смещение для прокрутки
    int screenRows = 24, screenCols = 80;
    bool dirty = false;
    std::string filename;
    std::vector<Row> rows;

    EditorConfig();
    void openFile(const std::string& filename);
    void saveFile();
    void processKeypress();
    void refreshScreen();
    void moveCursor(KeyAction key);
    void insertChar(char c);
    void deleteChar();
    void setStatusMessage(const std::string& message);
    void search(const std::string& query);


};

#endif 
