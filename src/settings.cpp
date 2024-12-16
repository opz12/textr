#include "settings.h"


/* Возвращает синглтон Settings, к которому имеют доступ все части
   приложения.
 */
Settings *Settings::instance()
{
    static Settings singleton;
    return &singleton;
}


// Простая обертка для setValue из QSettings.
void Settings::setValue(const QString &key, const QVariant &value)
{
    settings.setValue(key, value);
}


// Обертка для value из QSettings.
QVariant Settings::value(const QString &key, const QVariant &defaultValue) {
    return settings.value(key, defaultValue);
}


/* Применяет заданную настройку с использованием переданной функции-обработчика.
   Предполагается, что обработчик захватывает контекст вызова (указатель this).
 */
void Settings::apply(QVariant setting, std::function<void(QVariant)> handler)
{
    if (!setting.isNull())
    {
        handler(setting);
    }
}
