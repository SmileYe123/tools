#pragma once

#include <QObject>
#include <QList>
#include "plugins/base.h"

class PluginManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager();

    void loadPlugins();
    QList<BasePlugin*> plugins() const;

private:
    QList<BasePlugin*> m_plugins;
};
