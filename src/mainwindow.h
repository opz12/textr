#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    QLabel *wordCountLabel;
    QLabel *charCountLabel;
    QLabel *linesCountLabel;
    QString currentFile;
    QString fileText;
    QString outsideFileName;
    QString textfontcolor;
    QString menubarbcolor = "";
    QString menubarfcolor = "";
    QString widgetbcolor = "";
    QString texteditbcolor = "";
    QString statusbarbcolor = "";
    QString statusbarfcolor = "";
    bool isFresh = true;
    bool wordsOn = false;
    bool charOn = false;
    bool linesOn = false;
    Qt::WindowFlags flags = this->windowFlags();
    QDateTime dateTime;
    QFont tfont;
    QFont mfont;
    QFont sfont;
    QString tfontfamily;
    QString tfontsize;
    QString tfontweight;
    QString tfontstyle;
    QString tfontdecoration;
    QString mfontfamily;
    QString mfontsize;
    QString mfontweight;
    QString mfontstyle;
    QString mfontdecoration;
    QString sfontfamily;
    QString sfontsize;
    QString sfontweight;
    QString sfontstyle;
    QString sfontdecoration;
    bool wordwrapchecked;
    QString texteditfontcolor;
    QString sessiontfontcolor = "";

    void SaveSettings();
    void LoadSettings();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_Save_As_triggered();
    void on_action_Paste_triggered();
    void on_action_Copy_triggered();
    void on_action_Cut_triggered();
    void on_action_Undo_triggered();
    void on_action_Redo_triggered();
    void on_action_Exit_triggered();
    void on_action_Font_triggered();
    void on_action_Color_triggered();
    void on_action_Delete_triggered();
    void on_action_Select_All_triggered();
    void on_actionPrint_triggered();
    void on_action_New_Window_triggered();
    void on_action_gui_style_Fusion_triggered();
    void on_action_gui_style_Windows_old_triggered();
    void on_action_gui_style_Windows_new_triggered();

};


#endif // MAINWINDOW_H
