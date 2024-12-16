#include "finddialog.h"
#include <QHBoxLayout>

// реализация диалогового окна поиска и замены текста - это и есть finddialog

// Инициализирует этот объект FindDialog.
FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{

    initializeWidgets();

    // Гарантирует, что поле ввода получает фокус всякий раз, когда диалог становится активным окном
    setFocusProxy(findLineEdit);

    // Настраивает все виджеты и макеты
    initializeLayout();

    setWindowTitle(tr("Find and Replace"));

    connect(findNextButton, SIGNAL(clicked()), this, SLOT(on_findNextButton_clicked()));
    connect(replaceButton, SIGNAL(clicked()), this, SLOT(on_replaceOperation_initiated()));
    connect(replaceAllButton, SIGNAL(clicked()), this, SLOT(on_replaceOperation_initiated()));
}


// Выполняет все необходимые операции очистки памяти.
FindDialog::~FindDialog()
{
    delete findLabel;
    delete replaceLabel;
    delete findLineEdit;
    delete replaceLineEdit;
    delete findNextButton;
    delete replaceButton;
    delete replaceAllButton;
    delete caseSensitiveCheckBox;
    delete wholeWordsCheckBox;
    delete findHorizontalLayout;
    delete replaceHorizontalLayout;
    delete optionsLayout;
    delete verticalLayout;
}


// Инициализирует все дочерние виджеты, такие как метки, флажки и кнопки.
void FindDialog::initializeWidgets()
{
    findLabel = new QLabel(tr("Find what:    "));
    replaceLabel = new QLabel(tr("Replace with:"));
    findLineEdit = new QLineEdit();
    replaceLineEdit = new QLineEdit();
    findNextButton = new QPushButton(tr("&Find next"));
    replaceButton = new QPushButton(tr("&Replace"));
    replaceAllButton = new QPushButton(tr("&Replace all"));
    caseSensitiveCheckBox = new QCheckBox(tr("&Match case"));
    wholeWordsCheckBox = new QCheckBox(tr("&Whole words"));
}


/* Определяет макет FindDialog и добавляет виджеты
   в соответствующие дочерние макеты.
 */
void FindDialog::initializeLayout()
{
    findHorizontalLayout = new QHBoxLayout();
    replaceHorizontalLayout = new QHBoxLayout();
    optionsLayout = new QHBoxLayout();
    verticalLayout = new QVBoxLayout();

    verticalLayout->addLayout(findHorizontalLayout);
    verticalLayout->addLayout(replaceHorizontalLayout);
    verticalLayout->addLayout(optionsLayout);

    findHorizontalLayout->addWidget(findLabel);
    findHorizontalLayout->addWidget(findLineEdit);
    replaceHorizontalLayout->addWidget(replaceLabel);
    replaceHorizontalLayout->addWidget(replaceLineEdit);

    optionsLayout->addWidget(caseSensitiveCheckBox);
    optionsLayout->addWidget(wholeWordsCheckBox);
    optionsLayout->addWidget(findNextButton);
    optionsLayout->addWidget(replaceButton);
    optionsLayout->addWidget(replaceAllButton);

    setLayout(verticalLayout);
}


/* Вызывается, когда пользователь нажимает кнопку "Найти далее". Если запрос пуст, информирует пользователя.
   В противном случае отправляет соответствующий сигнал для начала поиска с учетом всех критериев.
 */
void FindDialog::on_findNextButton_clicked()
{
    QString query = findLineEdit->text();

    if (query.isEmpty())
    {
        QMessageBox::information(this, tr("Empty Field"), tr("Please enter a query."));
        return;
    }

    bool caseSensitive = caseSensitiveCheckBox->isChecked();
    bool wholeWords = wholeWordsCheckBox->isChecked();
    emit(startFinding(query, caseSensitive, wholeWords));
}


/* Вызывается, когда пользователь нажимает кнопку "Заменить" или "Заменить все". Отправляет соответствующий
   сигнал (startReplacing или startReplacingAll), передавая всю необходимую информацию для поиска и замены.
 */
void FindDialog::on_replaceOperation_initiated()
{
    QString what = findLineEdit->text();

    if (what.isEmpty())
    {
        QMessageBox::information(this, tr("Empty Field"), tr("Please enter a query."));
        return;
    }

    QString with = replaceLineEdit->text();
    bool caseSensitive = caseSensitiveCheckBox->isChecked();
    bool wholeWords = wholeWordsCheckBox->isChecked();
    bool replace = sender() == replaceButton;

    if (replace)
    {
        emit(startReplacing(what, with, caseSensitive, wholeWords));
    }
    else
    {
        emit(startReplacingAll(what, with, caseSensitive, wholeWords));
    }

}

