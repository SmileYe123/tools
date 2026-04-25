#pragma once

#include <QSettings>
#include <QString>
#include <QVariant>

class Config
{
public:
    static Config& getInstance()
    {
        static Config instance;
        return instance;
    }

    QString get(const QString& key, const QString& defaultValue = "")
    {
        return m_settings.value(key, defaultValue).toString();
    }

    void set(const QString& key, const QVariant& value)
    {
        m_settings.setValue(key, value);
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

private:
    Config() : m_settings("MultiTool.ini", QSettings::IniFormat) {}

    QSettings m_settings;
};
