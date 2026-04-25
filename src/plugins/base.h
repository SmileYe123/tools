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
    void setName(const QString& name) { m_name = name; }
    void setIcon(const QString& icon) { m_icon = icon; }
    void setDescription(const QString& desc) { m_description = desc; }
    void setCategory(const QString& category) { m_category = category; }

private:
    QString m_name;
    QString m_icon;
    QString m_description;
    QString m_category;
};
