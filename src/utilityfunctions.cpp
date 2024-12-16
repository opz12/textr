#include "utilityfunctions.h"
#include <QStack>
#include <QtDebug>
#include <QQueue>



/* Запускает диалоговое окно с вопросом "Да" или "Нет" в контексте
   указанного родительского виджета. Запрашивает у пользователя выбор.
 */
QMessageBox::StandardButton Utility::promptYesOrNo(QWidget *parent, QString title, QString prompt)
{
    QMessageBox asker;
    asker.setEscapeButton(QMessageBox::StandardButton::Cancel);
    return asker.question(parent, title, prompt, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
}


/* Возвращает true, если в переданной строке нужно вставить закрывающую
   скобку для создания сбалансированного выражения, и false в противном случае.
 */
bool Utility::codeBlockNotClosed(QString context, QChar startDelimiter, QChar endDelimiter)
{
    QStack<char> codeBlockStartDelimiters;

    for (int i = 0; i < context.length(); i++)
    {
        char character = context.at(i).toLatin1();

        if (character == startDelimiter)
        {
            codeBlockStartDelimiters.push(character);
        }

        else if (character == endDelimiter && !codeBlockStartDelimiters.empty())
        {
            codeBlockStartDelimiters.pop();
        }
    }

    return !codeBlockStartDelimiters.empty();
}

