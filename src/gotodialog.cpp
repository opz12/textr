#include "gotodialog.h"
#include <QMessageBox>


// инициализация gotodialog
GotoDialog::GotoDialog(QWidget *parent) : QDialog(parent)
{
    gotoLabel = new QLabel(tr("Line: "));
    gotoLineEdit = new QLineEdit();
    gotoButton = new QPushButton("Go");
    layout = new QHBoxLayout();

    layout->addWidget(gotoLabel);
    layout->addWidget(gotoLineEdit);
    layout->addWidget(gotoButton);

    setLayout(layout);
    setWindowTitle(tr("Go To"));

    // Гарантирует, что поле ввода получает фокус при открытии диалогового окна
    setFocusProxy(gotoLineEdit);

    connect(gotoButton, SIGNAL(clicked()), this, SLOT(on_gotoButton_clicked()));
}


// Выполняет все необходимые операции очистки памяти.
GotoDialog::~GotoDialog()
{
    delete gotoLabel;
    delete gotoLineEdit;
    delete gotoButton;
    delete layout;
}


// Вызывается, когда пользователь нажимает кнопку "Go".
void GotoDialog::on_gotoButton_clicked()
{
    QString line = gotoLineEdit->text();

    if (line.isEmpty())
    {
        QMessageBox::information(this, tr("Go"), tr("Must enter a line number."));
        return;
    }

    emit(gotoLine(line.toInt()));
}
