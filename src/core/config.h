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
        return m_settings->value(key, defaultValue).toString();
    }

    void set(const QString& key, const QVariant& value)
    {
        m_settings->setValue(key, value);
    }

private:
    Config()
    {
        m_settings = new QSettings("MultiTool.ini", QSettings::IniFormat);
    }

    QSettings* m_settings;
};
