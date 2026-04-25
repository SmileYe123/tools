#pragma once

#include <QObject>
#include <QWidget>
#include <QString>

class BasePlugin : public QObject
{
    Q_OBJECT

public:
    explicit BasePlugin(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~BasePlugin() = default;

    virtual QWidget* createWidget(QWidget* parent = nullptr) = 0;

    QString name() const { return m_name; }
    QString icon() const { return m_icon; }
    QString description() const { return m_description; }
    QString category() const { return m_category; }

protected:
    QString m_name;
    QString m_icon;
    QString m_description;
    QString m_category;
};
