#include "tabbededitor.h"
#include "utilityfunctions.h"
#include <QFont>
#include <QFontDialog>
#include <QtDebug>

// tabbededitor - нужен для работы с несколькими вкладками

// Инициализирует этот TabbedEditor с одной вкладкой Editor.
TabbedEditor::TabbedEditor(QWidget *parent) : QTabWidget(parent)
{
    add(new Editor());
    installEventFilter(this);
    setMovable(true);
}


/* Добавляет указанный Editor в виде новой вкладки.
   tab - объект Editor, который нужно добавить в виде вкладки этого виджета
 */
void TabbedEditor::add(Editor *tab)
{
    QTabWidget::addTab(tab, tab->getFileName());
    setCurrentWidget(tab);
}


/* Возвращает указатель на текущую вкладку Editor этого TabbedEditor.
 */
Editor* TabbedEditor::currentTab() const
{
    return qobject_cast<Editor*>(widget(currentIndex()));
}


/* Возвращает вкладку по указанному индексу (от 0 до count() - 1).
 */
Editor* TabbedEditor::tabAt(int index) const
{
    if (index < 0 || index >= count())
    {
        return nullptr;
    }

    return qobject_cast<Editor*>(widget(index));
}


/* Возвращает вектор всех вкладок Editor, которые содержит этот TabbedEditor.
 */
QVector<Editor*> TabbedEditor::tabs() const
{
    QVector<Editor*> tabs;

    for (int i = 0; i < count(); i++)
    {
        tabs.push_back(tabAt(i));
    }

    return tabs;
}


/* Возвращает вектор всех вкладок Editor, которые не сохранены.
 */
QVector<Editor*> TabbedEditor::unsavedTabs() const
{
    QVector<Editor*> unsavedTabs;

    for (int i = 0; i < count(); i++)
    {
        Editor *tab = tabAt(i);

        if (tab->isUnsaved())
        {
            unsavedTabs.push_back(tab);
        }
    }

    return unsavedTabs;
}


/* Запускает QFontDialog для выбора шрифта пользователем.
 */
void TabbedEditor::promptFontSelection()
{
    bool userChoseFont;
    QFont newFont = QFontDialog::getFont(&userChoseFont, currentTab()->getFont(), this);

    if (!userChoseFont) return;

    QMessageBox::StandardButton tabSelection = Utility::promptYesOrNo(this, tr("Font change"),
                                                                      tr("Apply to all open and future tabs?"));

    // Применить шрифт ко всем вкладкам
    if (tabSelection == QMessageBox::Yes)
    {
        for (Editor *tab : tabs())
        {
            tab->setFont(newFont, QFont::Monospace, true, Editor::NUM_CHARS_FOR_TAB);
        }
    }

    // Применить шрифт только к текущей вкладке
    else if (tabSelection == QMessageBox::No)
    {
        currentTab()->setFont(newFont, QFont::Monospace, true, Editor::NUM_CHARS_FOR_TAB);
    }

    // Если пользователь нажал отмену
    else
    {
        return;
    }
}


/* Применяет перенос слов ко всем вкладкам или только к текущей, в зависимости от выбора пользователя.
 */
bool TabbedEditor::applyWordWrapping(bool shouldWrap)
{
    if (numTabs() == 1)
    {
        currentTab()->toggleWrapMode(shouldWrap);
        return true;
    }

    // Все нижеописанное применяется, если открыто > 1 вкладка
    QMessageBox::StandardButton tabSelection = Utility::promptYesOrNo(this, tr("Word wrapping"),
                                                                      tr("Apply to all open and future tabs?"));

    // Применить перенос слов ко всем вкладкам
    if (tabSelection == QMessageBox::Yes)
    {
        for (Editor *tab : tabs())
        {
            tab->toggleWrapMode(shouldWrap);
        }

        return true;
    }

    // Применить перенос слов только к текущей вкладке
    else if (tabSelection == QMessageBox::No)
    {
        currentTab()->toggleWrapMode(shouldWrap);
        return true;
    }

    // Если пользователь нажал отмену
    else
    {
        return false;
    }
}


/* Применяет автоотступ ко всем вкладкам или только к текущей, в зависимости от выбора пользователя.
 * Возвращает true, если форматирование было применено, и false, если операция была отменена.
 */
bool TabbedEditor::applyAutoIndentation(bool shouldAutoIndent)
{
    if (numTabs() == 1)
    {
        currentTab()->toggleAutoIndent(shouldAutoIndent);
        return true;
    }

    // Все нижеописанное применяется, если открыто > 1 вкладка

    QMessageBox::StandardButton tabSelection = Utility::promptYesOrNo(this, tr("Auto indentation"),
                                                                      tr("Apply to all open and future tabs?"));

    // Применить автоотступ ко всем вкладкам
    if (tabSelection == QMessageBox::Yes)
    {
        for (Editor *tab : tabs())
        {
            tab->toggleAutoIndent(shouldAutoIndent);
        }

        return true;
    }

    // Применить автоотступ только к текущей вкладке
    else if (tabSelection == QMessageBox::No)
    {
        currentTab()->toggleAutoIndent(shouldAutoIndent);
        return true;
    }

    // Если пользователь нажал отмену
    else
    {
        return false;
    }
}


/* Обрабатывает события ввода. В основном используется для того, чтобы позволить пользователю
 * переключаться между вкладками с помощью ctrl + num и ctrl + tab.
 */
bool TabbedEditor::eventFilter(QObject* obj, QEvent* event)
{
    bool isKeyPress = event->type() == QEvent::KeyPress;

    if (isKeyPress)
    {
        QKeyEvent *keyInfo = static_cast<QKeyEvent*>(event);
        int key = keyInfo->key();

        if (keyInfo->modifiers() == Qt::ControlModifier)
        {
            // Ctrl + num = переход на вкладку с этим номером
            if (key >= Qt::Key_1 && key <= Qt::Key_9)
            {
                setCurrentWidget(tabAt(key - Qt::Key_1));
                return true;
            }

            // Ctrl + tab = переключиться на следующую вкладку
            else if (key == Qt::Key_T)
            {
                int newTabIndex = (currentIndex() + 1) % count();
                setCurrentWidget(tabAt(newTabIndex));
                return true;
            }
        }
        else
        {
            return QObject::eventFilter(obj, event);
        }
    }

    return QObject::eventFilter(obj, event);
}
