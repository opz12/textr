#include "mainwindow.h"
#include "utilityfunctions.h"
#include "ui_mainwindow.h"
#include "settings.h"                   // хранит состояние приложение
#include <QtDebug>
#include <QtPrintSupport/QPrinter>      // печать
#include <QtPrintSupport/QPrintDialog>  // печать
#include <QFileDialog>                  // открытие файла/сохранение
#include <QFile>                        // директории файлов, IO
#include <QTextStream>                  // файл IO
#include <QStandardPaths>               // базовая открытая директория
#include <QDateTime>                    // нынешнее время
#include <QApplication>
#include <QShortcut>


// Устанавливает главное окно приложения (наследников + виджеты)

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    readSettings();

    // Используется для гарантии того, чтобы проверялся только один язык
    languageGroup = new QActionGroup(this);
    languageGroup->setExclusive(true);
    languageGroup->addAction(ui->actionC_Lang);
    languageGroup->addAction(ui->actionCPP_Lang);
    languageGroup->addAction(ui->actionJava_Lang);
    languageGroup->addAction(ui->actionPython_Lang);
    connect(languageGroup, SIGNAL(triggered(QAction*)), this, SLOT(on_languageSelected(QAction*)));
    // Рамка языковой метки
    setupLanguageOnStatusBar();

    // Настройка диалогового окна поиска
    findDialog = new FindDialog();
    findDialog->setParent(this, Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);

    // Настройка диалогового окна перехода
    gotoDialog = new GotoDialog();
    gotoDialog->setParent(this, Qt::Tool | Qt::MSWindowsFixedSizeDialogHint);

    // Настройка редактора с вкладками
    tabbedEditor = ui->tabWidget;
    tabbedEditor->setTabsClosable(true);

    // Добавил metric reporter и смоделировал переключение вкладок
    metricReporter = new MetricReporter();
    ui->statusBar->addPermanentWidget(metricReporter);
    on_currentTabChanged(0);

    // Подключил сигналы редактора с вкладками к их обработчикам
    connect(tabbedEditor, SIGNAL(currentChanged(int)), this, SLOT(on_currentTabChanged(int)));
    connect(tabbedEditor, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    // Подключил сигналы действий к их обработчикам
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(on_actionSaveTriggered()));
    connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(on_actionSaveTriggered()));
    connect(ui->actionReplace, SIGNAL(triggered()), this, SLOT(on_actionFind_triggered()));

    // Приходится добавлять этот ярлык вручную, потому что мы не можем определить его с помощью графического редактора
    QShortcut *tabCloseShortcut = new QShortcut(QKeySequence("Ctrl+W"), this);
    QObject::connect(tabCloseShortcut, SIGNAL(activated()), this, SLOT(closeTabShortcut()));

    // Для переноса слов и автоматического отступа
    matchFormatOptionsToEditorDefaults();

    mapMenuLanguageOptionToLanguageType();
    mapFileExtensionsToLanguages();
    appendShortcutsToToolbarTooltips();
}


/* Гарантирует, что доступные для проверки параметры меню форматирования, такие как автоматический отступ
   и перенос слов соответствуют ранее сохраненным значениям по умолчанию для класса редактора.
   Чекнуть конструктор для использования.
 */
void MainWindow::matchFormatOptionsToEditorDefaults()
{
    QAction *autoIndent = ui->actionAuto_Indent;
    editor->autoIndentEnabled ? autoIndent->setChecked(true) : autoIndent->setChecked(false);

    QAction *wordWrap = ui->actionWord_Wrap;
    editor->lineWrapMode ? wordWrap->setChecked(true) : wordWrap->setChecked(false);
}


/* Обновляет параметры меню форматирования (например, перенос слов, автоматический отступ) в соответствии с
   настройками текущего выбранного редактора. Смотреть раздел onCurrentTabChanged для использования.
 */
void MainWindow::updateFormatMenuOptions()
{
    ui->actionWord_Wrap->setChecked(editor->textIsWrapped());
    ui->actionAuto_Indent->setChecked(editor->textIsAutoIndented());
}


/* Инициализирует языковую метку и добавляет ее во фрейм
   это устанавливается в качестве виджета в крайнем левом углу строки состояния.
 */
void MainWindow::setupLanguageOnStatusBar()
{
    languageLabel = new QLabel("Language: not selected");
    QFrame *langFrame = new QFrame();
    QHBoxLayout *langLayout = new QHBoxLayout();
    langLayout->addWidget(languageLabel);
    langFrame->setLayout(langLayout);
    ui->statusBar->addWidget(langFrame);
}


/* Сопоставляет каждый языковой параметр меню (из выпадающего списка Формат) с соответствующим ему
   Вводить язык для удобства.
 */
void MainWindow::mapMenuLanguageOptionToLanguageType()
{
    menuActionToLanguageMap[ui->actionC_Lang] = Language::C;
    menuActionToLanguageMap[ui->actionCPP_Lang] = Language::CPP;
    menuActionToLanguageMap[ui->actionJava_Lang] = Language::Java;
    menuActionToLanguageMap[ui->actionPython_Lang] = Language::Python;
}


// Сопоставляет известные расширения файлов с языками, поддерживаемыми редактором.
void MainWindow::mapFileExtensionsToLanguages()
{
    extensionToLanguageMap.insert("cpp", Language::CPP);
    extensionToLanguageMap.insert("h", Language::CPP);
    extensionToLanguageMap.insert("c", Language::C);
    extensionToLanguageMap.insert("java", Language::Java);
    extensionToLanguageMap.insert("py", Language::Python);
}


void MainWindow::appendShortcutsToToolbarTooltips()
{
    for (QAction* action : ui->mainToolBar->actions())
    {
        QString tooltip = action->toolTip() + " (" + action->shortcut().toString() + ")";
        action->setToolTip(tooltip);
    }
}

// Выполняет все необходимые операции по очистке памяти для динамически выделяемых объектов.
MainWindow::~MainWindow()
{
    delete languageLabel;
    delete languageGroup;
    delete ui;
}


/* Вызывается, когда пользователь выбирает язык в главном меню. Устанавливает для текущего языка значение
   этот язык используется внутри текущего редактора с вкладками.
 */
void MainWindow::on_languageSelected(QAction* languageAction)
{
    Language language = menuActionToLanguageMap[languageAction];
    selectProgrammingLanguage(language);
}


/* При наличии перечисления языков эта функция проверяет соответствующий параметр радио в меню Формат > Язык
   меню. Используется on_currentTabChanged для отображения выбранного языка текущей вкладки.
 */
void MainWindow::triggerCorrespondingMenuLanguageOption(Language lang)
{
    switch (lang)
    {
    case (Language::C):
        if (!ui->actionC_Lang->isChecked())
        {
            ui->actionC_Lang->trigger();
        }
        break;

    case (Language::CPP):
        if (!ui->actionCPP_Lang->isChecked())
        {
            ui->actionCPP_Lang->trigger();
        }
        break;

    case (Language::Java):
        if (!ui->actionJava_Lang->isChecked())
        {
            ui->actionJava_Lang->trigger();
        }
        break;

    case (Language::Python):
        if (!ui->actionPython_Lang->isChecked())
        {
            ui->actionPython_Lang->trigger();
        }
        break;

    default: return;
    }
}


/* Использует расширение файла, чтобы определить, на какой язык (если таковой имеется) он должен быть переведен.
   Если расширение не соответствует ни одному из поддерживаемых языков или
   если файл не имеет расширения, то для языка устанавливается значение Language::None.
 */
void MainWindow::setLanguageFromExtension()
{
    QString fileName = editor->getFileName();
    int indexOfDot = fileName.indexOf('.');

    if (indexOfDot == -1)
    {
        selectProgrammingLanguage(Language::None);
        return;
    }

    QString fileExtension = fileName.mid(indexOfDot + 1);

    bool extensionSupported = extensionToLanguageMap.find(fileExtension) != extensionToLanguageMap.end();

    if (!extensionSupported)
    {
        selectProgrammingLanguage(Language::None);
        return;
    }

    selectProgrammingLanguage(extensionToLanguageMap[fileExtension]);
}


/* Оболочка для всей общей логики, которая должна запускаться всякий раз, когда
   выбран определенный язык для использования на определенной вкладке. Запускает соответствующий пункт меню.
 */
void MainWindow::selectProgrammingLanguage(Language language)
{
    if (language == editor->getProgrammingLanguage())
    {
        return;
    }

    editor->setProgrammingLanguage(language);
    languageLabel->setText(toString(language));
    triggerCorrespondingMenuLanguageOption(language);
}


/* Отключает все сигналы, зависящие от кэшированного редактора/вкладки.
   Используется в основном при изменении текущего редактора (например, при открытии новой вкладки).
 */
void MainWindow::disconnectEditorDependentSignals()
{
    disconnect(findDialog, SIGNAL(startFinding(QString, bool, bool)), editor, SLOT(find(QString, bool, bool)));
    disconnect(findDialog, SIGNAL(startReplacing(QString, QString, bool, bool)), editor, SLOT(replace(QString, QString, bool, bool)));
    disconnect(findDialog, SIGNAL(startReplacingAll(QString, QString, bool, bool)), editor, SLOT(replaceAll(QString, QString, bool, bool)));
    disconnect(gotoDialog, SIGNAL(gotoLine(int)), editor, SLOT(goTo(int)));
    disconnect(editor, SIGNAL(findResultReady(QString)), findDialog, SLOT(onFindResultReady(QString)));
    disconnect(editor, SIGNAL(gotoResultReady(QString)), gotoDialog, SLOT(onGotoResultReady(QString)));

    disconnect(editor, SIGNAL(wordCountChanged(int)), metricReporter, SLOT(updateWordCount(int)));
    disconnect(editor, SIGNAL(charCountChanged(int)), metricReporter, SLOT(updateCharCount(int)));
    disconnect(editor, SIGNAL(lineCountChanged(int, int)), metricReporter, SLOT(updateLineCount(int, int)));
    disconnect(editor, SIGNAL(columnCountChanged(int)), metricReporter, SLOT(updateColumnCount(int)));
    disconnect(editor, SIGNAL(fileContentsChanged()), this, SLOT(updateTabAndWindowTitle()));

    disconnect(editor, SIGNAL(undoAvailable(bool)), this, SLOT(toggleUndo(bool)));
    disconnect(editor, SIGNAL(redoAvailable(bool)), this, SLOT(toggleRedo(bool)));
    disconnect(editor, SIGNAL(copyAvailable(bool)), this, SLOT(toggleCopyAndCut(bool)));
}


/* Соединяет все сигналы и интервалы, зависящие от кэшированного редактора/вкладки.
   Используется в основном при изменении текущего редактора (например, при открытии новой вкладки).
 */
void MainWindow::reconnectEditorDependentSignals()
{
    connect(findDialog, SIGNAL(startFinding(QString, bool, bool)), editor, SLOT(find(QString, bool, bool)));
    connect(findDialog, SIGNAL(startReplacing(QString, QString, bool, bool)), editor, SLOT(replace(QString, QString, bool, bool)));
    connect(findDialog, SIGNAL(startReplacingAll(QString, QString, bool, bool)), editor, SLOT(replaceAll(QString, QString, bool, bool)));
    connect(gotoDialog, SIGNAL(gotoLine(int)), editor, SLOT(goTo(int)));
    connect(editor, SIGNAL(findResultReady(QString)), findDialog, SLOT(onFindResultReady(QString)));
    connect(editor, SIGNAL(gotoResultReady(QString)), gotoDialog, SLOT(onGotoResultReady(QString)));

    connect(editor, SIGNAL(wordCountChanged(int)), metricReporter, SLOT(updateWordCount(int)));
    connect(editor, SIGNAL(charCountChanged(int)), metricReporter, SLOT(updateCharCount(int)));
    connect(editor, SIGNAL(lineCountChanged(int, int)), metricReporter, SLOT(updateLineCount(int, int)));
    connect(editor, SIGNAL(columnCountChanged(int)), metricReporter, SLOT(updateColumnCount(int)));
    connect(editor, SIGNAL(fileContentsChanged()), this, SLOT(updateTabAndWindowTitle()));

    connect(editor, SIGNAL(undoAvailable(bool)), this, SLOT(toggleUndo(bool)));
    connect(editor, SIGNAL(redoAvailable(bool)), this, SLOT(toggleRedo(bool)));
    connect(editor, SIGNAL(copyAvailable(bool)), this, SLOT(toggleCopyAndCut(bool)));
}


/* Вызывается при каждом изменении текущей вкладки в редакторе с вкладками. Устанавливает текущий редактор главного окна,
   повторно подключает все соответствующие сигналы и обновляет окно.
 */
void MainWindow::on_currentTabChanged(int index)
{
    // Происходит, когда закрывается последняя вкладка редактора с вкладками
    if (index == -1)
    {
        return;
    }

    // Примечание: редактор будет иметь значение nullptr только при первом запуске, поэтому в этом случае edge это будет пропущено
    if (editor != nullptr)
    {
        disconnectEditorDependentSignals();
    }

    editor = tabbedEditor->currentTab();
    reconnectEditorDependentSignals();
    editor->setFocus(Qt::FocusReason::TabFocusReason);

    Language tabLanguage = editor->getProgrammingLanguage();

    // Если на этой вкладке был установлен язык программирования, активируйте соответствующую опцию
    if (tabLanguage != Language::None)
    {
        triggerCorrespondingMenuLanguageOption(tabLanguage);
    }
    else
    {
        // Если выбран язык меню, но для текущей вкладки язык не установлен, снимите флажок с пункта меню
        if (languageGroup->checkedAction())
        {
            languageGroup->checkedAction()->setChecked(false);
        }
    }

    // Обновить язык, отображаемый в строке состояния
    languageLabel->setText(toString(tabLanguage));

    // Обновите действия в главном окне, чтобы отразить доступные действия на текущей вкладке
    toggleRedo(editor->redoAvailable());
    toggleUndo(editor->undoAvailable());
    toggleCopyAndCut(editor->textCursor().hasSelection());

    updateFormatMenuOptions();


    // Нам необходимо обновить эту информацию вручную для внесения изменений в вкладку
    DocumentMetrics metrics = editor->getDocumentMetrics();
    updateTabAndWindowTitle();
    metricReporter->updateWordCount(metrics.wordCount);
    metricReporter->updateCharCount(metrics.charCount);
    metricReporter->updateLineCount(metrics.currentLine, metrics.totalLines);
    metricReporter->updateColumnCount(metrics.currentColumn);
}


// Запускает диалоговое окно поиска, если оно еще не отображается, и устанавливает его фокус.
void MainWindow::launchFindDialog()
{
    if (findDialog->isHidden())
    {
        findDialog->show();
        findDialog->activateWindow();
        findDialog->raise();
        findDialog->setFocus();
    }
}


// Запускает диалоговое окно "Перейти к", если оно еще не отображается, и устанавливает его фокус.
void MainWindow::launchGotoDialog()
{
    if (gotoDialog->isHidden())
    {
        gotoDialog->show();
        gotoDialog->activateWindow();
        gotoDialog->raise();
        gotoDialog->setFocus();
    }
}


/* Обновляет название вкладки и заголовок главного окна приложения, чтобы
   они отражали текущий открытый документ.
 */
void MainWindow::updateTabAndWindowTitle()
{
    QString tabTitle = editor->getFileName();
    QString windowTitle = tabTitle;

    if (editor->isUnsaved())
    {
        tabTitle += " *";
        windowTitle += " [Unsaved]";
    }

    tabbedEditor->setTabText(tabbedEditor->currentIndex(), tabTitle);
    setWindowTitle(windowTitle + " - textr");
}


/* Запускает диалоговое окно с запросом у пользователя, хочет ли он сохранить текущий файл.
   Если пользователь выберет "Нет" или закроет диалоговое окно, файл не будет сохранен.
   В противном случае, если они выберут "Да", файл будет сохранен.
 */
QMessageBox::StandardButton MainWindow::askUserToSave()
{
    QString fileName = editor->getFileName();

    return Utility::promptYesOrNo(this, "Unsaved changes", tr("Do you want to save the changes to ") + fileName + tr("?"));
}


/* Вызывается, когда пользователь выбирает новую опцию в меню или на панели инструментов (или использует сочетание клавиш Ctrl+N).
   Добавляет новую вкладку в редактор.
 */
void MainWindow::on_actionNew_triggered()
{
    tabbedEditor->add(new Editor());
}


/* Вызывается, когда пользователь выбирает опцию Сохранить или Сохранить как в меню или на панели инструментов
   или использует сочетание клавиш Ctrl+S). В случае успеха содержимое текстового редактора сохраняется на диске с использованием
   имени файла, указанного пользователем. Если текущий документ никогда не сохранялся или
   пользователь выбрал Сохранить как, программа предложит пользователю указать имя и каталог для файла.
   Возвращает значение true, если файл был сохранен, и значение false в противном случае.
 */
bool MainWindow::on_actionSaveTriggered()
{
    bool saveAs = sender() == ui->actionSave_As;
    QString currentFilePath = editor->getCurrentFilePath();

    // Если пользователь нажал Сохранить как или пользователь нажал Сохранить, но текущий документ так и не был сохранен на диск
    if (saveAs || currentFilePath.isEmpty())
    {
        // Заголовок, который будет использоваться для сохранения диалогового окна
        QString saveDialogWindowTitle = saveAs ? tr("Save As") : tr("Save");

        // Попробуйте ввести правильный путь к файлу
        QString filePath = QFileDialog::getSaveFileName(this, saveDialogWindowTitle);

        // Ничего не предпринимайте, если пользователь передумает и нажмет кнопку Отмена
        if (filePath.isNull())
        {
            return false;
        }
        editor->setCurrentFilePath(filePath);
    }

    // Попытайтесь создать файловый дескриптор с заданным путем
    QFile file(editor->getCurrentFilePath());
    if (!file.open(QIODevice::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Warning", "Cannot save file: " + file.errorString());
        return false;
    }

    ui->statusBar->showMessage("Document saved", 2000);

    // Сохраните содержимое редактора на диск и закройте файловый дескриптор
    QTextStream out(&file);
    QString editorContents = editor->toPlainText();
    out << editorContents;
    file.close();

    editor->setModifiedState(false);
    updateTabAndWindowTitle();
    setLanguageFromExtension();

    return true;
}


/* Вызывается, когда пользователь выбирает опцию "Открыть" в меню или на панели инструментов
   (или использует сочетание клавиш Ctrl+O). Если в текущем документе есть несохраненные изменения, он сначала
   запрашивает пользователя, хочет ли он их сохранить. В любом случае, он запускает диалоговое окно
   позволяющее пользователю выбрать файл, который он хочет открыть. При успешном открытии пути к текущему файлу в редакторе
   устанавливается путь к файлу, который был открыт, и обновляется состояние приложения.
 */
void MainWindow::on_actionOpen_triggered()
{
    // Используется для перехода на новую вкладку, если уже есть открытый документ
    bool openInCurrentTab = editor->isUntitled() && !editor->isUnsaved();

    QString openedFilePath;
    QString lastUsedDirectory = settings->value(DEFAULT_DIRECTORY_KEY).toString();

    if (lastUsedDirectory.isEmpty())
    {
        openedFilePath = QFileDialog::getOpenFileName(this, tr("Open"), DEFAULT_DIRECTORY);
    }
    else
    {
        openedFilePath = QFileDialog::getOpenFileName(this, tr("Open"), lastUsedDirectory);
    }

    // Ничего не предпринимает, если пользователь нажмет кнопку Отмена
    if (openedFilePath.isNull())
    {
        return;
    }

    // Обновить недавно использованный каталог
    QDir currentDirectory;
    settings->setValue(DEFAULT_DIRECTORY_KEY, currentDirectory.absoluteFilePath(openedFilePath));

    // Попытка создать файловый дескриптор для файла по заданному пути
    QFile file(openedFilePath);
    if (!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Warning", "Cannot save file: " + file.errorString());
        return;
    }

    // Прочитать содержимое файла в редакторе и закройте файловый дескриптор
    QTextStream in(&file);
    QString documentContents = in.readAll();

    if (!openInCurrentTab)
    {
        tabbedEditor->add(new Editor());
    }
    editor->setCurrentFilePath(openedFilePath);
    editor->setPlainText(documentContents);
    file.close();

    editor->setModifiedState(false);
    updateTabAndWindowTitle();
    setLanguageFromExtension();
}


/* Вызывается, когда пользователь выбирает опцию печати в меню или на панели инструментов (или использует сочетание клавиш Ctrl+P).
   Позволяет пользователю распечатать содержимое текущего документа.
 */
void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer;
    printer.setPrinterName(tr("Document printer"));
    QPrintDialog printDialog(&printer, this);

    if (printDialog.exec() != QPrintDialog::Rejected)
    {
        editor->print(&printer);
        ui->statusBar->showMessage("Printing", 2000);
    }
}


/* Вызывается, когда пользователь пытается закрыть вкладку в редакторе (или использует Ctrl+W). Позволяет пользователю
   сохранить содержимое вкладки, если оно не сохранено. Закрывает вкладку, если только файл не является несохраненным
   и пользователь отказывается от сохранения. Возвращает значение true, если вкладка была закрыта, и значение false в противном случае.
 */
bool MainWindow::closeTab(Editor *tabToClose)
{
    Editor *currentTab = editor;
    bool closingCurrentTab = (tabToClose == currentTab);

    // Позволяет пользователю видеть, какую вкладку он закрывает, если она не является текущей
    if (!closingCurrentTab)
    {
        tabbedEditor->setCurrentWidget(tabToClose);
    }

    // Не закрывает вкладку сразу, если на ней есть несохраненное содержимое
    if (tabToClose->isUnsaved())
    {
        QMessageBox::StandardButton selection = askUserToSave();

        if (selection == QMessageBox::StandardButton::Yes)
        {
            bool fileSaved = on_actionSaveTriggered();

            if (!fileSaved)
            {
                return false;
            }
        }

        else if (selection == QMessageBox::Cancel)
        {
            return false;
        }
    }

    int indexOfTabToClose = tabbedEditor->indexOf(tabToClose);
    tabbedEditor->removeTab(indexOfTabToClose);

    // Если закрыл последнюю вкладку, создает новую
    if (tabbedEditor->count() == 0)
    {
        on_actionNew_triggered();
    }

    // Возвращение к исходной вкладке, если пользователь закрывал другую
    if (!closingCurrentTab)
    {
        tabbedEditor->setCurrentWidget(currentTab);
    }

    return true;
}


/* Вызывается, когда пользователь выбирает опцию Выхода из меню. Позволяет пользователю
   сохранять любые несохраненные файлы перед завершением работы.
 */
void MainWindow::on_actionExit_triggered()
{
    QVector<Editor*> unsavedTabs = tabbedEditor->unsavedTabs();

    for (Editor *tab : unsavedTabs)
    {
        bool userClosedTab = closeTab(tab);

        if (!userClosedTab)
        {
            return;
        }
    }

    writeSettings();
    QApplication::quit();
}


/* Сохраняет основное состояние и настройки приложения, чтобы их можно было
   восстановить при следующем запуске приложения.
   См. readSettings и конструктор для большей информации.
 */
void MainWindow::writeSettings()
{
    settings->setValue(WINDOW_SIZE_KEY, size());
    settings->setValue(WINDOW_POSITION_KEY, pos());
    settings->setValue(WINDOW_STATUS_BAR, ui->statusBar->isVisible());
    settings->setValue(WINDOW_TOOL_BAR, ui->mainToolBar->isVisible());
}


// Считывает сохраненные настройки приложения и восстанавливает их.
void MainWindow::readSettings() {
    settings->apply(settings->value(WINDOW_SIZE_KEY, QSize(400, 400)),
                    [=](QVariant setting){ this->resize(setting.toSize()); });

    settings->apply(settings->value(WINDOW_POSITION_KEY, QPoint(200, 200)),
                    [=](QVariant setting){ this->move(setting.toPoint()); });

    settings->apply(settings->value(WINDOW_STATUS_BAR),
                    [=](QVariant setting) {
                        this->ui->statusBar->setVisible(qvariant_cast<bool>(setting));
                        this->ui->actionStatus_Bar->setChecked(qvariant_cast<bool>(setting));
                    });

    settings->apply(settings->value(WINDOW_TOOL_BAR),
                    [=](QVariant setting) {
                        this->ui->mainToolBar->setVisible(qvariant_cast<bool>(setting));
                        this->ui->actionTool_Bar->setChecked(qvariant_cast<bool>(setting));
                    });
}


// Вызывается, когда редактор переключает операцию отмены.
void MainWindow::toggleUndo(bool undoAvailable) {
    ui->actionUndo->setEnabled(undoAvailable);
}


// Вызывается, когда редактор переключает операцию повтора.
void MainWindow::toggleRedo(bool redoAvailable)
{
    ui->actionRedo->setEnabled(redoAvailable);
}


// Вызывается, когда пользователь выполняет операцию отмены.
void MainWindow::on_actionUndo_triggered() {
    if (ui->actionUndo->isEnabled())
    {
        editor->undo();
    }
}


// Вызывается, когда пользователь выполняет операцию повтора.
void MainWindow::on_actionRedo_triggered() {
    if (ui->actionRedo->isEnabled())
    {
        editor->redo();
    }
}


// Вызывается, когда редактор переключает операции копирования и вырезания.
void MainWindow::toggleCopyAndCut(bool copyCutAvailable) {
    ui->actionCopy->setEnabled(copyCutAvailable);
    ui->actionCut->setEnabled(copyCutAvailable);
}


// Вызывается, когда пользователь выполняет операцию вырезания
void MainWindow::on_actionCut_triggered() {
    if (ui->actionCut->isEnabled())
    {
        editor->cut();
    }
}


// Вызывается, когда пользователь выполняет операцию копирования.
void MainWindow::on_actionCopy_triggered() {
    if (ui->actionCopy->isEnabled())
    {
        editor->copy();
    }
}


// Вызывается, когда пользователь выполняет операцию вставки.
void MainWindow::on_actionPaste_triggered() {
    editor->paste();
}


/* Вызывается, когда пользователь явно выбирает опцию поиска в меню
   (или использует Ctrl+F). Запускает диалоговое окно, в котором пользователю предлагается ввести поисковый запрос.
 */
void MainWindow::on_actionFind_triggered() {
    launchFindDialog();
}


/* Вызывается, когда пользователь явно выбирает опцию Перейти в меню (или использует Ctrl+G).
   Запускает диалоговое окно перехода, в котором пользователю предлагается ввести номер строки, на которую он хочет перейти.
 */
void MainWindow::on_actionGo_To_triggered() {
    launchGotoDialog();
}


// Вызывается, когда пользователь явно выбирает опцию Выбрать все в меню (или использует сочетание клавиш Ctrl+A).
void MainWindow::on_actionSelect_All_triggered() {
    editor->selectAll();
}


// Вызывается, когда пользователь явно выбирает опцию "Время/дата" в меню (или использует клавишу F5).
void MainWindow::on_actionTime_Date_triggered()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    editor->insertPlainText(currentTime.toString());
}


// Вызывается, когда пользователь выбирает опцию шрифта в меню. Запускает диалоговое окно выбора шрифта.
void MainWindow::on_actionFont_triggered() {
    tabbedEditor->promptFontSelection();
}


// Вызывается, когда пользователь выбирает опцию автоматического отступа в меню Формат.
void MainWindow::on_actionAuto_Indent_triggered()
{
    bool shouldAutoIndent = ui->actionAuto_Indent->isChecked();
    bool autoIndentToggled = tabbedEditor->applyAutoIndentation(shouldAutoIndent);

    // Если пользователь отменил операцию, отмените проверку
    if (!autoIndentToggled)
    {
        ui->actionAuto_Indent->setChecked(!shouldAutoIndent);
    }
}


// Вызывается, когда пользователь выбирает опцию переноса слов в меню формат.
void MainWindow::on_actionWord_Wrap_triggered()
{
    tabbedEditor->applyWordWrapping(ui->actionWord_Wrap->isChecked());
}


/* Переключает видимость данного виджета. Предполагается, что этот
   виджет является частью главного окна. В противном случае эффект может быть незаметен.
 */
void MainWindow::toggleVisibilityOf(QWidget *widget) {
    bool opposite = !widget->isVisible();
    widget->setVisible(opposite);
}


// Переключает видимость строки состояния.
void MainWindow::on_actionStatus_Bar_triggered() {
    toggleVisibilityOf(ui->statusBar);
}


// Переключает видимость главной панели инструментов
void MainWindow::on_actionTool_Bar_triggered() {
    toggleVisibilityOf(ui->mainToolBar);
}


/* Переопределяет виртуальный метод QWidget closeEvent. Вызывается, когда пользователь пытается
   закрыть главное окно приложения обычным способом с помощью красного крестика. Позволяет
   пользователю сохранить все несохраненные файлы перед выходом из системы.
 */
void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    on_actionExit_triggered();
}
