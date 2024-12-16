#include "editor.h"
#include "linenumberarea.h"
#include "utilityfunctions.h"
#include "code_highlighters/chighlighter.h"
#include "code_highlighters/cpphighlighter.h"
#include "code_highlighters/javahighlighter.h"
#include "code_highlighters/pythonhighlighter.h"
#include <QPainter>
#include <QTextBlock>
#include <QFontDialog>
#include <QTextDocumentFragment>
#include <QPalette>
#include <QStack>
#include <QFileInfo>
#include <QtDebug>


const QColor Editor::LINE_COLOR = QColor(Qt::lightGray).lighter(125);


// Инициализация editor
Editor::Editor(QWidget *parent) : QPlainTextEdit (parent) {
    readSettings();
    document()->setModified(false);

    setProgrammingLanguage(Language::None);
    metrics = DocumentMetrics();
    lineNumberArea = new LineNumberArea(this);
    setFont(QFont("Courier", DEFAULT_FONT_SIZE), QFont::Monospace, true, NUM_CHARS_FOR_TAB);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth()));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(redrawLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
    connect(this, SIGNAL(textChanged()), this, SLOT(on_textChanged()));
    connect(this, SIGNAL(undoAvailable(bool)), this, SLOT(setUndoAvailable(bool)));
    connect(this, SIGNAL(redoAvailable(bool)), this, SLOT(setRedoAvailable(bool)));

    installEventFilter(this);
    updateLineNumberAreaWidth();
    on_cursorPositionChanged();
}


// чтобы не было утечек
Editor::~Editor() {
    delete lineNumberArea;
}


//reset в исходное состояние
void Editor::reset() {
    currentFilePath.clear();
    document()->setModified(false);
    setPlainText(QString());
}


// устанавливает текующий путь editor
// newPath - путь файла, который открыт в editor
void Editor::setCurrentFilePath(QString newPath) {
    currentFilePath = newPath;
    fileIsUntitled = false;
}


// извлекает имя файла из текущего пути к файлу
QString Editor::getFileNameFromPath() {
    if (currentFilePath.isEmpty())
    {
        fileIsUntitled = true;
        return "Untitled document";
    }

    QFileInfo fileInfo(currentFilePath);
    return fileInfo.fileName();
}


/* установка шрифта
   styleHint - используется для выбора подходящего семейства шрифтов по умолчанию, если указанный шрифт недоступен
   fixedPitch - при значении true используется monospace font (символы одинаковой ширины)
   tabStopWidth - желаемая ширина табуляции в пересчете на эквивалентное количество пробелов
 */
void Editor::setFont(QFont newFont, QFont::StyleHint styleHint, bool fixedPitch, int tabStopWidth) {
    font = newFont;
    font.setStyleHint(styleHint);
    font.setFixedPitch(fixedPitch);
    QPlainTextEdit::setFont(font);

    QFontMetrics metrics(font);
    setTabStopDistance(tabStopWidth * metrics.horizontalAdvance(' '));
}


// Установка ЯП в editor
void Editor::setProgrammingLanguage(Language language) {
    if (language == this->programmingLanguage)
    {
        return;
    }

    this->programmingLanguage = language;
    this->syntaxHighlighter = generateHighlighterFor(language);
}


/* Возвращает highlighter для соответ языка
   language - язык для которого должна быть подсветка
 */
Highlighter *Editor::generateHighlighterFor(Language language)
{
    QTextDocument *doc = document();

    switch (language)
    {
        case (Language::C): return new CHighlighter(doc);
        case (Language::CPP): return new CPPHighlighter(doc);
        case (Language::Java): return new JavaHighlighter(doc);
        case (Language::Python): return new PythonHighlighter(doc);
        default: return nullptr;
    }
}


/* Возвращает QTextDocument::FindFlags, предоставляющий все флаги, с помощью которых должен выполняться поиск.
   CaseSensitive - флаг, указывающий, должен ли поиск учитывать регистр результатов
   wholeWords - флажок, указывающий, следует ли искать совпадения по всему слову или по частям
 */
QTextDocument::FindFlags Editor::getSearchOptionsFromFlags(bool caseSensitive, bool wholeWords)
{
    QTextDocument::FindFlags searchOptions = QTextDocument::FindFlags();
    if (caseSensitive)
    {
        searchOptions |= QTextDocument::FindCaseSensitively;
    }
    if (wholeWords)
    {
        searchOptions |= QTextDocument::FindWholeWords;
    }
    return searchOptions;
}


/* Вызывается, когда объект findDialog дает сигнал queryReady. Инициирует
   фактический поиск в редакторе. Сначала ищет совпадение с текущей позиции до конца документа.
   Если ничего не найдено, поиск продолжается с начала документа, но останавливается,
   как только встречается первое совпадение (если таковое есть), найденное в предыдущих
   итерациях для текущего запроса.
   query - текст, который пользователь хочет найти
   caseSensitive - флаг, обозначающий, учитывать ли регистр при поиске
   wholeWords - флаг, обозначающий, искать ли только целые слова или частичные совпадения
 */
bool Editor::find(QString query, bool caseSensitive, bool wholeWords)
{
    // Сохраняем позицию курсора до начала поиска, чтобы вернуть её, если совпадений не будет найдено
    int cursorPositionBeforeCurrentSearch = textCursor().position();

    // Указываем параметры, с которыми будем выполнять поиск
    QTextDocument::FindFlags searchOptions = getSearchOptionsFromFlags(caseSensitive, wholeWords);

    // Поиск с текущей позиции до конца документа
    bool matchFound = QPlainTextEdit::find(query, searchOptions);

    // Если совпадений не найдено, ищем с начала документа
    if (!matchFound)
    {
        moveCursor(QTextCursor::Start);
        matchFound = QPlainTextEdit::find(query, searchOptions);
    }

    // Если совпадение найдено
    if (matchFound)
    {
        int foundPosition = textCursor().position();
        bool previouslyFound = searchHistory.previouslyFound(query);

        // Если совпадение найдено впервые, сохраняем первую позицию для текущего состояния документа
        // История поиска всегда сбрасывается при полном цикле обратно к первому совпадению или начале новой цепочки поиска
        if (!previouslyFound)
        {
            searchHistory.add(query, cursorPositionBeforeCurrentSearch, foundPosition);
        }
        // Если совпадение уже находилось ранее, проверяем, не вернулись ли мы к первому совпадению
        else
        {
            bool loopedBackToFirstMatch = foundPosition == searchHistory.firstFoundAt(query);

            if (loopedBackToFirstMatch)
            {
                // Это не новое совпадение, а повтор первого совпадения
                matchFound = false;

                // Возвращаем курсор на начальную позицию перед первым поиском для этого запроса
                int cursorPositionBeforeFirstSearch = searchHistory.cursorPositionBeforeFirstSearchFor(query);
                moveCursorTo(cursorPositionBeforeFirstSearch);

                // Очищаем историю поиска
                searchHistory.clear();

                // Сообщаем пользователю об отсутствии дальнейших совпадений
                emit(findResultReady("No more results found."));
            }
        }
    }
    else
    {
        // Возвращаем курсор на позицию до начала поиска
        moveCursorTo(cursorPositionBeforeCurrentSearch);

        // Сообщаем пользователю об отсутствии совпадений
        emit(findResultReady("No results found."));
    }

    return matchFound;
}


/* Вызывается, когда пользователь нажимает кнопку "Заменить" в FindDialog.
   what - строка для поиска и замены
   with - строка, которой заменить найденное совпадение
   caseSensitive - флаг, обозначающий, учитывать ли регистр при поиске
   wholeWords - флаг, обозначающий, искать ли только целые слова или частичные совпадения
 */
void Editor::replace(QString what, QString with, bool caseSensitive, bool wholeWords)
{
    bool found = find(what, caseSensitive, wholeWords);

    if (found)
    {
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        cursor.insertText(with);
        cursor.endEditBlock();
    }
}


/* Вызывается, когда пользователь нажимает кнопку "Заменить всё" в FindDialog.
   what - строка для поиска и замены
   with - строка, которой заменить все найденные совпадения
   caseSensitive - флаг, обозначающий, учитывать ли регистр при поиске
   wholeWords - флаг, обозначающий, искать ли только целые слова или частичные совпадения
 */
void Editor::replaceAll(QString what, QString with, bool caseSensitive, bool wholeWords) {
    // Оптимизация: не обновляем экран до завершения всех замен
    disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
    disconnect(this, SIGNAL(textChanged()), this, SLOT(on_textChanged()));

    // Начинаем поиск с самого начала документа
    moveCursorTo(0);

    // Выполняем начальный поиск; не полагаемся на наш пользовательский find
    QTextDocument::FindFlags searchOptions = getSearchOptionsFromFlags(caseSensitive, wholeWords);
    bool found = QPlainTextEdit::find(what, searchOptions);
    int replacements = 0;

    // Продолжаем замену, пока остаются совпадения
    QTextCursor cursor(document());
    cursor.beginEditBlock();
    while (found) {
        QTextCursor currentPosition = textCursor();
        currentPosition.insertText(with);
        replacements++;
        found = QPlainTextEdit::find(what, searchOptions);
    }
    cursor.endEditBlock();

    // Сообщение по завершении операции
    if (replacements == 0) {
        emit(findResultReady("No results found."));
    } else {
        emit(findResultReady("Document searched. Replaced " + QString::number(replacements) + " instances."));
    }

    // Восстанавливаем состояние и рассчитываем метрики за один проход
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
    connect(this, SIGNAL(textChanged()), this, SLOT(on_textChanged()));
    on_cursorPositionChanged();
    on_textChanged();
}


void Editor::goTo(int line)
{
    if (line > blockCount() || line < 1) {
        emit(gotoResultReady("Invalid line number."));
        return;
    }

    int beginningOfLine = document()->findBlockByLineNumber(line - 1).position();
    moveCursorTo(beginningOfLine);
}


/* Применяет заданное форматирование к выделенному тексту между двумя указанными индексами (включительно).
   Снимает форматирование со всего текста перед применением заданного форматирования, если флаг указан как true.
   startIndex - индекс, с которого должно начинаться форматирование
   endIndex - индекс, на котором форматирование должно завершиться
   format - формат, который нужно применить к тексту между startIndex и endIndex, включительно
   unformatAllFirst - флаг, обозначающий, нужно ли сначала снять форматирование со всего текста (по умолчанию false)
 */
void Editor::formatSubtext(int startIndex, int endIndex, QTextCharFormat format, bool unformatAllFirst) {
    QTextCursor cursorBeforeFormatting = textCursor();
    QTextCursor formatter = textCursor();

    if (unformatAllFirst)
    {
        QTextCursor unformatter = textCursor();
        unformatter.setPosition(0, QTextCursor::MoveAnchor);
        unformatter.setPosition(document()->toPlainText().length(), QTextCursor::KeepAnchor);
        unformatter.setCharFormat(defaultCharFormat);
        qDebug() << "Unformatting!";
        setTextCursor(unformatter);
    }

    formatter.setPosition(startIndex, QTextCursor::MoveAnchor);
    formatter.setPosition(endIndex, QTextCursor::KeepAnchor);
    formatter.setCharFormat(format);

    setTextCursor(formatter);
    setTextCursor(cursorBeforeFormatting);
}


// Устанавливает режим переноса строк редактора на указанное значение.
void Editor::setLineWrapMode(LineWrapMode lineWrapMode) {
    QPlainTextEdit::setLineWrapMode(lineWrapMode);
    this->lineWrapMode = lineWrapMode;
}


// Используется для включения/выключения режима автоотступов в редакторе.
void Editor::toggleAutoIndent(bool autoIndent) {
    autoIndentEnabled = autoIndent;

    // Обновляем настройку, чтобы она применялась к новым открытым вкладкам
    settings->setValue(AUTO_INDENT_KEY, autoIndentEnabled);
}


/* Используется для переключения режима переноса строк редактора (перенос или отсутствие переноса).
   wrap - флаг, обозначающий, следует ли переносить строки (true) или нет (false)
 */
void Editor::toggleWrapMode(bool wrap)
{
    if (wrap)
    {
        setLineWrapMode(LineWrapMode::WidgetWidth);
    }
    else
    {
        setLineWrapMode(LineWrapMode::NoWrap);
    }

    // Обновляем настройку, чтобы она применялась к новым открытым вкладкам
    settings->setValue(LINE_WRAP_KEY, lineWrapMode);
}


/* Вызывается всякий раз, когда содержимое текстового редактора изменяется. Сбрасывает
   историю поиска редактора, обновляет количество символов и выдает сигнал,
   чтобы другие объекты (например, MainWindow) могли обновить свои данные соответственно.
 */
void Editor::on_textChanged()
{
    searchHistory.clear();
    updateCharCount();
    updateWordCount();
    emit(fileContentsChanged());
}


// разбор документа для обновления количества слов. выдает сигнал с количеством слов.
void Editor::updateWordCount()
{
    metrics.wordCount = 0;
    QString documentContents = toPlainText().toUtf8();
    int documentLength = documentContents.length();
    QString currentWord = "";

    for (int i = 0; i < documentLength; i++)
    {
        // Преобразуем в unsigned char, чтобы избежать проблем с утверждениями в отладке
        unsigned char character = qvariant_cast<unsigned char>(documentContents[i].toLatin1());

        // Символ новой строки
        if (character == '\n')
        {
            // Особый случай: новая строка после слова
            if (!currentWord.isEmpty())
            {
                metrics.wordCount++;
                currentWord.clear();
            }
        }
        // Все остальные допустимые символы
        else
        {
            // Буквенно-цифровой символ
            if (isalnum(character))
            {
                currentWord += qvariant_cast<char>(character);
            }
            // Пробельный символ (исключая новую строку, которая обрабатывается отдельно выше)
            else if (isspace(character))
            {
                // Пробел после слова означает завершение слова
                if (!currentWord.isEmpty())
                {
                    metrics.wordCount++;
                    currentWord.clear();
                }
            }
        }
    }

    // Например, если мы остановились на слове, которое всё ещё набирается, его нужно учесть
    if (!currentWord.isEmpty())
    {
        metrics.wordCount++;
        currentWord.clear();
    }

    emit(wordCountChanged(metrics.wordCount));
}



// Обновляет и выдает количество символов.
void Editor::updateCharCount()
{
    metrics.charCount = toPlainText().length();
    emit(charCountChanged(metrics.charCount));
}


// Возвращает уровень отступа текущей строки текста.
int Editor::indentationLevelOfCurrentLine()
{
    QTextCursor originalCursor = textCursor();
    moveCursorToStartOfCurrentLine();

    QString documentContents = document()->toPlainText();
    int indentationLevel = 0;

    int index = textCursor().position();
    if (index >= documentContents.length())
    {
        index--;
    }

    while (index < documentContents.length())
    {
        if (documentContents.at(index) == '\t')
        {
            indentationLevel++;
            index++;
        }
        else
        {
            break;
        }
    }

    setTextCursor(originalCursor);
    return indentationLevel;
}


// Вставляет указанное количество табуляций в документ.
void Editor::insertTabs(int numTabs)
{
    for (int i = 0; i < numTabs; i++)
    {
        insertPlainText("\t");
    }
}


// Перемещает курсор к началу текущей строки.
void Editor::moveCursorToStartOfCurrentLine()
{
    QTextCursor cursor = textCursor();

    do
    {
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
    }
    while (metrics.currentColumn != 1);
}


/* Увеличивает отступ выделенного текста, если курсор выделил текст.
   Возвращает true, если удалось, и false в противном случае.
 */
void Editor::indentSelection(QTextDocumentFragment selection)
{
    QString text = selection.toPlainText();

    text.insert(0, '\t');
    for (int i = 1; i < text.length(); i++)
    {
        // Вставляет табуляцию после каждой новой строки
        if (text.at(i) == '\n' && i + 1 < text.length())
        {
            text.insert(i + 1, '\t');
        }
    }

    // Заменяет выделенный текст новым текстом с табуляцией
    insertPlainText(text);
}


// перемещает курсор текста этого редактора в указанную позицию в документе.
void Editor::moveCursorTo(int positionInText)
{
    QTextCursor newCursor = textCursor();
    newCursor.setPosition(positionInText);
    setTextCursor(newCursor);
}


// Обрабатывает нажатие клавиши Enter.
bool Editor::handleEnterKeyPress()
{
    QString documentContents = document()->toPlainText();
    int indexToLeftOfCursor = textCursor().position() - 1;

    // Граничные случаи
    if (documentContents.length() < 1 ||
        indexToLeftOfCursor < 0 ||
        indexToLeftOfCursor >= documentContents.length())
    {
        return false;
    }

    int currentIndent = indentationLevelOfCurrentLine();
    QChar characterToLeftOfCursor = documentContents.at(indexToLeftOfCursor);

    // Проверяет, нажал ли пользователь ENTER сразу после начала блока кода, например, открывающей фигурной скобки в C++
    if (syntaxHighlighter && characterToLeftOfCursor == syntaxHighlighter->getCodeBlockStartDelimiter())
    {
        QChar codeBlockStartDelimiter = syntaxHighlighter->getCodeBlockStartDelimiter();
        QChar codeBlockEndDelimiter = syntaxHighlighter->getCodeBlockEndDelimiter();

        insertPlainText("\n");
        if (autoIndentEnabled)
        {
            insertTabs(currentIndent + 1);
        }

        // Примечание: некоторые языки, такие как Python, не имеют завершающего разделителя блока кода.
        if (codeBlockEndDelimiter != NULL &&
            Utility::codeBlockNotClosed(documentContents, codeBlockStartDelimiter, codeBlockEndDelimiter))
        {
            insertPlainText("\n");
            insertTabs(currentIndent);
            insertPlainText(codeBlockEndDelimiter);

            // Устанавливает курсор сразу после вложенной табуляции
            moveCursorTo(textCursor().position() - 2 - currentIndent);
        }

        return true;
    }

    // Если пользователь нажал ENTER после чего-либо другого, переносит его на следующую строку, сохраняя текущий уровень отступа
    else
    {
        insertPlainText("\n");
        if (autoIndentEnabled)
        {
            insertTabs(currentIndent);
        }

        return true;
    }
}


/* Определяет всю логику, которая должна происходить при нажатии пользователем клавиши табуляции.
   В данный момент редактор проверяет только то, была ли клавиша табуляции нажата, когда текст был выделен.
   Если так, текст будет сдвинут с отступом.
 */
bool Editor::handleTabKeyPress()
{
    if (textCursor().hasSelection())
    {
        indentSelection(textCursor().selection());
        return true;
    }

    return false;
}



/* Используется для обработки случаев, когда нажата клавиша Enter после открывающей фигурной скобки
   или клавиша табуляции используется для выделенного текста.
 */
bool Editor::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        int key = static_cast<QKeyEvent*>(event)->key();

        if (key == Qt::Key_Enter || key == Qt::Key_Return)
        {
            return handleEnterKeyPress();
        }
        else if (key == Qt::Key_Tab)
        {
            return handleTabKeyPress();
        }
        else
        {
            return QObject::eventFilter(obj, event);
        }
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
}


// Сохраняет текущие настройки редактора, чтобы их можно было использовать при следующем запуске.
void Editor::writeSettings()
{
    settings->setValue(LINE_WRAP_KEY, lineWrapMode);
    settings->setValue(AUTO_INDENT_KEY, autoIndentEnabled);
}


// Загружает ранее сохраненные настройки редактора.
void Editor::readSettings()
{
    settings->apply(settings->value(LINE_WRAP_KEY),
                    [=](QVariant setting){
                        LineWrapMode wrap = qvariant_cast<LineWrapMode>(setting);
                        this->setLineWrapMode(wrap);
                    }
                    );

    settings->apply(settings->value(AUTO_INDENT_KEY),
                    [=](QVariant setting){
                        this->autoIndentEnabled = qvariant_cast<bool>(setting);
                    }
                    );
}


/* ------------------------------------------------------------
   Все функции ниже этой строки используются для lineNumberArea
  -----------------------------------------------------------
 */


/* возвращает ширину области номеров строк редактора, вычисляя количество цифр в номере
   последней строки, умножая это значение на максимальную ширину любой цифры,
   и добавляет фиксированный размер отступа.
 */
int Editor::getLineNumberAreaWidth()
{
    int lastLineNumber = blockCount();
    int numDigitsInLastLine = QString::number(lastLineNumber).length();
    int maxWidthOfAnyDigit = fontMetrics().horizontalAdvance(QLatin1Char('9')); // 9 выбрана произвольно
    return numDigitsInLastLine * maxWidthOfAnyDigit + lineNumberAreaPadding;
}


/* вызывается, когда пользователь изменяет количество блоков (абзацев) в документе.
   обновляет левое поле редактора так, чтобы его ширина соответствовала самой широкой строке номера.
 */
void Editor::updateLineNumberAreaWidth()
{
    setViewportMargins(getLineNumberAreaWidth() + lineNumberAreaPadding, 0, 0, 0);
}


// Вызывается, когда область просмотра редактора прокручивается. Перерисовывает область номеров строк соответственно.
void Editor::redrawLineNumberArea(const QRect &rectToBeRedrawn, int numPixelsScrolledVertically)
{
    if (numPixelsScrolledVertically != 0)
    {
        lineNumberArea->scroll(0, numPixelsScrolledVertically);
    }
    else
    {
        lineNumberArea->update(0, rectToBeRedrawn.y(), lineNumberArea->width(), rectToBeRedrawn.height());
    }

    if (rectToBeRedrawn.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth();
    }
}


// Вызывается, когда редактор изменяет размер. Изменяет размер области номеров строк соответственно.
void Editor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), getLineNumberAreaWidth(), cr.height()));
}


// Вызывается, когда курсор изменяет позицию.
void Editor::on_cursorPositionChanged()
{
    highlightCurrentLine();
    updateLineCount();
    updateColumnCount();
}


// Обновляет и выдает количество строк (текущих и общих).
void Editor::updateLineCount()
{
    metrics.currentLine = textCursor().blockNumber() + 1;
    metrics.totalLines = document()->lineCount();
    emit(lineCountChanged(metrics.currentLine, metrics.totalLines));
}


// Обновляет и выдает количество колонок.
void Editor::updateColumnCount()
{
    metrics.currentColumn = textCursor().positionInBlock() + 1;
    emit(columnCountChanged(metrics.currentColumn));
}


// Подсвечивает текущую строку. См. вызов on_cursorPositionChanged().
void Editor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(LINE_COLOR);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}


/* См. linenumberarea.h для вызова. Перебирает каждый блок (абзац/строку) в
   редакторе и рисует соответствующие номера строк в lineNumberArea.
 */
void Editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qvariant_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qvariant_cast<int>(blockBoundingRect(block).height());

    // перебирает каждый блок (абзац) и рисует его соответствующий номер
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString lineNumber = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, lineNumber);
        }

        block = block.next();
        top = bottom;
        bottom = top + qvariant_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}


