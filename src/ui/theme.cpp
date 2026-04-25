#include "theme.h"
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

Theme::Theme(QObject* parent)
    : QObject(parent), m_currentTheme(Light)
{
    m_accentColor = "#3B82F6";
}

Theme::Theme(ThemeType type, QObject* parent)
    : QObject(parent), m_currentTheme(type)
{
    setTheme(type);
}

void Theme::setTheme(ThemeType type)
{
    m_currentTheme = type;
    switch (type) {
        case Dark:
            m_accentColor = "#007ACC";
            break;
        case Light:
            m_accentColor = "#3B82F6";
            break;
        case OceanBlue:
            m_accentColor = "#00ACC1";
            break;
        case ForestGreen:
            m_accentColor = "#4CAF50";
            break;
    }
}

Theme::ThemeType Theme::currentTheme() const
{
    return m_currentTheme;
}

QString Theme::getAccentColor() const
{
    return m_accentColor;
}

QStringList Theme::availableThemes()
{
    return {
        QCoreApplication::translate("Theme", "浅色"),
        QCoreApplication::translate("Theme", "深色"),
        QCoreApplication::translate("Theme", "海洋蓝"),
        QCoreApplication::translate("Theme", "森林绿")
    };
}

QString Theme::themeName(ThemeType type)
{
    switch (type) {
        case Light: return QCoreApplication::translate("Theme", "浅色");
        case Dark: return QCoreApplication::translate("Theme", "深色");
        case OceanBlue: return QCoreApplication::translate("Theme", "海洋蓝");
        case ForestGreen: return QCoreApplication::translate("Theme", "森林绿");
    }
    return QCoreApplication::translate("Theme", "浅色");
}

QString Theme::getStyleSheet() const
{
    QString qssPath;
    switch (m_currentTheme) {
        case Dark:
        case OceanBlue:
        case ForestGreen:
            qssPath = ":/themes/dark.qss";
            break;
        case Light:
            qssPath = ":/themes/light.qss";
            break;
    }

    QFile file(qssPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }

    QTextStream stream(&file);
    QString styleSheet = stream.readAll();
    file.close();

    return styleSheet;
}
