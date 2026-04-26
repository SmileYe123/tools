#pragma once

#include <QSettings>
#include <QString>
#include <QVariant>
#include <QMutex>

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
        QMutexLocker locker(&m_mutex);
        return m_settings.value(key, defaultValue).toString();
    }

    void set(const QString& key, const QVariant& value)
    {
        QMutexLocker locker(&m_mutex);
        m_settings.setValue(key, value);
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

private:
    Config() : m_settings("MultiTool.ini", QSettings::IniFormat) {}

    QSettings m_settings;
    QMutex m_mutex;
};
