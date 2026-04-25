#include "plugin_manager.h"
#include "plugins/color_picker/color_picker.h"
#include "plugins/csv_tool/csv_tool.h"
#include "plugins/hash_tool/hash_tool.h"
#include "plugins/image_tool/image_tool.h"
#include "plugins/json_formatter/json_formatter.h"
#include "plugins/regex_tool/regex_tool.h"
#include "plugins/sql_formatter/sql_formatter.h"
#include "plugins/text_tools/text_tools.h"
#include "plugins/timestamp_tool/timestamp_tool.h"

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager()
{
    qDeleteAll(m_plugins);
    m_plugins.clear();
}

void PluginManager::loadPlugins()
{
    m_plugins.append(new ColorPickerPlugin(this));
    m_plugins.append(new CsvToolPlugin(this));
    m_plugins.append(new HashToolPlugin(this));
    m_plugins.append(new ImageToolPlugin(this));
    m_plugins.append(new JsonFormatterPlugin(this));
    m_plugins.append(new RegexToolPlugin(this));
    m_plugins.append(new SqlFormatterPlugin(this));
    m_plugins.append(new TextToolPlugin(this));
    m_plugins.append(new TimestampToolPlugin(this));
}

QList<BasePlugin*> PluginManager::plugins() const
{
    return m_plugins;
}
