#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QFile>
#include <QSaveFile>
#include <QLabel>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QMainWindow>
#include <QCloseEvent>
#include <QFontDialog>
#include <QColorDialog>
#include <QTextEdit>
#include <QPrinter>
#include <QPrintDialog>
#include <QDesktopServices>
#include <QRegularExpression>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QFile>
#include <QSaveFile>
#include <QLabel>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QMainWindow>
#include <QCloseEvent>
#include <QFontDialog>
#include <QColorDialog>
#include <QTextEdit>
#include <QPrinter>
#include <QPrintDialog>
#include <QDesktopServices>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    QApplication::setStyle("fusion");
    setWindowTitle("Untitled - textr");
    fileText = ui->textEdit->toPlainText();
    
    ui->action_Undo->setDisabled(true);
    ui->action_Redo->setDisabled(true);
    ui->action_Copy->setDisabled(true);
    ui->action_Cut->setDisabled(true);
    ui->action_Delete->setDisabled(true);
    ui->action_Select_All->setDisabled(true);
    ui->action_Uppercase->setDisabled(true);
    ui->action_Lowercase->setDisabled(true);
    ui->action_Word_Wrap->setCheckable(true);
    ui->action_Vertical->setCheckable(true);
    ui->action_Horizontal->setCheckable(true);
    ui->action_Both->setCheckable(true);
    ui->action_None->setCheckable(true);
    ui->action_Both->setChecked(true);
    ui->action_Box->setCheckable(true);
    ui->action_Panel->setCheckable(true);
    ui->action_Win_Panel->setCheckable(true);
    ui->action_Styled_Panel->setCheckable(true);
    ui->action_No_Frame->setCheckable(true);
    ui->action_Styled_Panel->setChecked(true);
    ui->action_statusBar_On->setCheckable(true);
    ui->action_statusBar_Off->setCheckable(true);
    ui->action_statusBar_Off->setChecked(true);
    ui->action_Word_Counter_On->setCheckable(true);
    ui->action_Word_Counter_Off->setCheckable(true);
    ui->action_Word_Counter_Off->setChecked(true);
    ui->action_Character_Counter_On->setCheckable(true);
    ui->action_Character_Counter_Off->setCheckable(true);
    ui->action_Character_Counter_Off->setChecked(true);
    ui->action_Lines_Counter_On->setCheckable(true);
    ui->action_Lines_Counter_Off->setCheckable(true);
    ui->action_Lines_Counter_Off->setChecked(true);
    ui->action_statusBar_On->setChecked(false);
    ui->action_statusBar_Off->setChecked(true);
    ui->action_Word_Counter_On->setDisabled(true);
    ui->action_Word_Counter_Off->setDisabled(true);
    ui->action_Character_Counter_On->setDisabled(true);
    ui->action_Character_Counter_Off->setDisabled(true);
    ui->action_Lines_Counter_On->setDisabled(true);
    ui->action_Lines_Counter_Off->setDisabled(true);
    ui->action_statusBar_Font->setDisabled(true);
    ui->action_statusBar_Font_Color->setDisabled(true);
    ui->action_statusBar_Appearance->setDisabled(true);
    ui->action_statusBar_Reset_to_default->setDisabled(true);
    ui->statusbar->hide();
    
    linesCountLabel = new QLabel(this);
    charCountLabel = new QLabel(this);
    wordCountLabel = new QLabel(this);
    
    ui->statusbar->addPermanentWidget(linesCountLabel);
    ui->statusbar->addPermanentWidget(charCountLabel);
    ui->statusbar->addPermanentWidget(wordCountLabel);
    
    LoadSettings();
    
    QStringList arguments = QCoreApplication::arguments();
    if(arguments.length() > 1)
    {
        outsideFileName = arguments[1];
        outsideNotepadOpen();
    }
    
    QColor textfcolor = ui->textEdit->textColor();
    textfontcolor = textfcolor.name();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SaveSettings()
{
    QSettings setting("wiiun", "textr");

    setting.beginGroup("MainWindow");
    setting.setValue("geometry", saveGeometry());
    setting.setValue("windowState", saveState());
    setting.endGroup();

    setting.beginGroup("TextEdit");
    setting.setValue("textedit.word.wrap", ui->action_Word_Wrap->isChecked());
    setting.setValue("textedit.stylesheet", ui->textEdit->styleSheet());
    setting.setValue("textedit.font.color", ui->textEdit->textColor());
    setting.setValue("textedit.frame.shape.box", ui->action_Box->isChecked());
    setting.setValue("textedit.frame.shape.panel", ui->action_Panel->isChecked());
    setting.setValue("textedit.frame.shape.win.panel", ui->action_Win_Panel->isChecked());
    setting.setValue("textedit.frame.shape.styled.panel", ui->action_Styled_Panel->isChecked());
    setting.setValue("textedit.frame.shape.no.frame", ui->action_No_Frame->isChecked());
    setting.setValue("textedit.scroll.bar.vertical", ui->action_Vertical->isChecked());
    setting.setValue("textedit.scroll.bar.horizontal", ui->action_Horizontal->isChecked());
    setting.setValue("textedit.scroll.bar.both", ui->action_Both->isChecked());
    setting.setValue("textedit.scroll.bar.none", ui->action_None->isChecked());
    setting.setValue("tfontfamily", tfontfamily);
    setting.setValue("tfontsize", tfontsize);
    setting.setValue("tfontweight", tfontweight);
    setting.setValue("tfontstyle", tfontstyle);
    setting.setValue("tfontdecoration", tfontdecoration);
    setting.setValue("texteditbcolor", texteditbcolor);
    setting.endGroup();

    setting.beginGroup("MenuBar");
    setting.setValue("menubar.stylesheet", ui->menubar->styleSheet());
    setting.setValue("mfontfamily", mfontfamily);
    setting.setValue("mfontsize", mfontsize);
    setting.setValue("mfontweight", mfontweight);
    setting.setValue("mfontstyle", mfontstyle);
    setting.setValue("mfontdecoration", mfontdecoration);
    setting.setValue("menubarbcolor", menubarbcolor);
    setting.setValue("menubarfcolor", menubarfcolor);
    setting.endGroup();

    setting.beginGroup("StatusBar");
    setting.setValue("statusbar.on", ui->action_statusBar_On->isChecked());
    setting.setValue("statusbar.off", ui->action_statusBar_Off->isChecked());
    setting.setValue("statusbar.stylesheet", ui->statusbar->styleSheet());
    setting.setValue("statusbar.word.counter.on", ui->action_Word_Counter_On->isChecked());
    setting.setValue("statusbar.word.counter.off", ui->action_Word_Counter_Off->isChecked());
    setting.setValue("statusbar.char.counter.on", ui->action_Character_Counter_On->isChecked());
    setting.setValue("statusbar.char.counter.off", ui->action_Character_Counter_Off->isChecked());
    setting.setValue("statusbar.lines.counter.on", ui->action_Lines_Counter_On->isChecked());
    setting.setValue("statusbar.lines.counter.off", ui->action_Lines_Counter_Off->isChecked());
    setting.setValue("sfontfamily", sfontfamily);
    setting.setValue("sfontsize", sfontsize);
    setting.setValue("sfontweight", sfontweight);
    setting.setValue("sfontstyle", sfontstyle);
    setting.setValue("sfontdecoration", sfontdecoration);
    setting.setValue("statusbarbcolor", statusbarbcolor);
    setting.setValue("statusbarfcolor", statusbarfcolor);
    setting.endGroup();
}

void MainWindow::LoadSettings()
{
    QSettings setting("wiiun", "textr");

    setting.beginGroup("MainWindow");
    restoreGeometry(setting.value("geometry").toByteArray());
    restoreState(setting.value("windowState").toByteArray());
    setting.endGroup();

    setting.beginGroup("TextEdit");
    wordwrapchecked = setting.value("textedit.word.wrap").toBool();

    if (wordwrapchecked == true)
    {
        ui->action_Word_Wrap->setChecked(true);
        ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);

        if (ui->action_Horizontal->isChecked())
        {
            ui->action_Horizontal->setChecked(false);
            ui->textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            ui->textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            ui->action_None->setChecked(true);
        }

        if (ui->action_Both->isChecked())
        {
            ui->action_Both->setChecked(false);
            ui->textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            ui->textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            ui->action_Vertical->setChecked(true);
        }

        ui->action_Horizontal->setEnabled(false);
        ui->action_Both->setEnabled(false);
    }
    else
    {
        ui->action_Word_Wrap->setChecked(false);
        ui->textEdit->setLineWrapMode(QTextEdit::NoWrap);
        ui->action_Horizontal->setEnabled(true);
        ui->action_Both->setEnabled(true);
    }


    QString texteditstylesheet = setting.value("textedit.stylesheet").toString();
    ui->textEdit->setStyleSheet(texteditstylesheet);
    texteditfontcolor = setting.value("textedit.font.color").toString();
    ui->textEdit->setTextColor(texteditfontcolor);

    bool isframeboxchecked = setting.value("textedit.frame.shape.box").toBool();

    if (isframeboxchecked == true)
    {
        on_action_Box_triggered();
    }

    bool isframepanelchecked = setting.value("textedit.frame.shape.panel").toBool();

    if (isframepanelchecked == true)
    {
        on_action_Panel_triggered();
    }

    bool isframewinpanelchecked = setting.value("textedit.frame.shape.win.panel").toBool();

    if (isframewinpanelchecked == true)
    {
        on_action_Win_Panel_triggered();
    }

    bool isframestyledpanelchecked = setting.value("textedit.frame.shape.styled.panel").toBool();

    if (isframestyledpanelchecked == true)
    {
        on_action_Styled_Panel_triggered();
    }

    bool isframenoframechecked = setting.value("textedit.frame.shape.no.frame").toBool();

    if (isframenoframechecked == true)
    {
        on_action_No_Frame_triggered();
    }

    bool scrollbarvertical = setting.value("textedit.scroll.bar.vertical").toBool();

    if (scrollbarvertical == true)
    {
        on_action_Vertical_triggered();
    }

    bool scrollbarhorizontal = setting.value("textedit.scroll.bar.horizontal").toBool();

    if (scrollbarhorizontal == true)
    {
        on_action_Horizontal_triggered();
    }

    bool scrollbarboth = setting.value("textedit.scroll.bar.both").toBool();

    if (scrollbarboth == true)
    {
        on_action_Both_triggered();
    }

    bool scrollbarnone = setting.value("textedit.scroll.bar.none").toBool();

    if (scrollbarnone == true)
    {
        on_action_None_triggered();
    }
    tfontfamily = setting.value("tfontfamily").toString();
    tfontsize = setting.value("tfontsize").toString();
    tfontweight = setting.value("tfontweight").toString();
    tfontstyle = setting.value("tfontstyle").toString();
    tfontdecoration = setting.value("tfontdecoration").toString();
    texteditbcolor = setting.value("texteditbcolor").toString();
    setting.endGroup();

    setting.beginGroup("MenuBar");
    QString menubarstylesheet = setting.value("menubar.stylesheet").toString();
    ui->menubar->setStyleSheet(menubarstylesheet);
    mfontfamily = setting.value("mfontfamily").toString();
    mfontsize = setting.value("mfontsize").toString();
    mfontweight = setting.value("mfontweight").toString();
    mfontstyle = setting.value("mfontstyle").toString();
    mfontdecoration = setting.value("mfontdecoration").toString();
    menubarbcolor = setting.value("menubarbcolor").toString();
    menubarfcolor = setting.value("menubarfcolor").toString();
    setting.endGroup();

    setting.beginGroup("StatusBar");
    bool statusbaron = setting.value("statusbar.on").toBool();

    if (statusbaron == true)
    {
        on_action_statusBar_On_triggered();
    }

    bool statusbaroff = setting.value("statusbar.off").toBool();

    if (statusbaroff == true)
    {
        on_action_statusBar_Off_triggered();
    }

    QString statusbarstylesheet = setting.value("statusbar.stylesheet").toString();
    ui->statusbar->setStyleSheet(statusbarstylesheet);

    bool statusbarwordcounteron = setting.value("statusbar.word.counter.on").toBool();

    if (statusbarwordcounteron == true)
    {
        on_action_Word_Counter_On_triggered();
    }

    bool statusbarwordcounteroff = setting.value("statusbar.word.counter.off").toBool();

    if (statusbarwordcounteroff == true)
    {
        on_action_Word_Counter_Off_triggered();
    }

    bool statusbarcharcounteron = setting.value("statusbar.char.counter.on").toBool();

    if (statusbarcharcounteron == true)
    {
        on_action_Character_Counter_On_triggered();
    }

    bool statusbarcharcounteroff = setting.value("statusbar.char.counter.off").toBool();

    if (statusbarcharcounteroff == true)
    {
        on_action_Character_Counter_Off_triggered();
    }

    bool statusbarlinescounteron = setting.value("statusbar.lines.counter.on").toBool();

    if (statusbarlinescounteron == true)
    {
        on_action_Lines_Counter_On_triggered();
    }

    bool statusbarlinescounteroff = setting.value("statusbar.lines.counter.off").toBool();

    if (statusbarlinescounteroff == true)
    {
        on_action_Lines_Counter_Off_triggered();
    }

    sfontfamily = setting.value("sfontfamily").toString();
    sfontsize = setting.value("sfontsize").toString();
    sfontweight = setting.value("sfontweight").toString();
    sfontstyle = setting.value("sfontstyle").toString();
    sfontdecoration = setting.value("sfontdecoration").toString();
    statusbarbcolor = setting.value("statusbarbcolor").toString();
    statusbarfcolor = setting.value("statusbarfcolor").toString();
    setting.endGroup();
}



void MainWindow::on_action_Undo_triggered()
{
    ui->textEdit->undo();
}

void MainWindow::on_action_Redo_triggered()
{
    ui->textEdit->redo();
}

void MainWindow::on_textEdit_undoAvailable(bool b)
{
    if (b == true)
    {
        ui->action_Undo->setEnabled(true);
    }
    else
    {
        ui->action_Undo->setDisabled(true);
    }
}

void MainWindow::on_textEdit_redoAvailable(bool b)
{
    if (b == true)
    {
        ui->action_Redo->setEnabled(true);
    }
    else
    {
        ui->action_Redo->setDisabled(true);
    }
}

void MainWindow::on_action_Paste_triggered()
{
    ui->textEdit->paste();
}

void MainWindow::on_action_Copy_triggered()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    if(cursor.hasSelection())
    {
        ui->textEdit->copy();
    }
}

void MainWindow::on_action_Cut_triggered()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    if(cursor.hasSelection())
    {
        ui->textEdit->cut();
    }
}

void MainWindow::on_action_Delete_triggered()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    if(cursor.hasSelection()) {
        cursor.deleteChar();
    }
}

void MainWindow::on_action_Exit_triggered()
{
    QWidget::close();
}


