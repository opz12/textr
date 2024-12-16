#ifndef Settings_H
#define Settings_H
#include <QSettings>
#include <QString>
#include <QVariant>


class Settings
{
public:
    static Settings *instance();
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
    void apply(QVariant setting, std::function<void(QVariant)> handler);

// Singleton
private:
    Settings(){}
    Settings(const Settings& other);
    Settings &operator=(const Settings& other);
    QSettings settings;
};

#endif // Settings_H
