#include "theme.h"
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

Theme::Theme(QObject* parent)
    : QObject(parent), m_currentTheme(Light)
{
    initLightTheme();
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
            initDarkTheme();
            break;
        case Light:
            initLightTheme();
            break;
        case OceanBlue:
            initOceanBlueTheme();
            break;
        case ForestGreen:
            initForestGreenTheme();
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
        QCoreApplication::translate("Theme", "深色"),
        QCoreApplication::translate("Theme", "浅色"),
        QCoreApplication::translate("Theme", "海洋蓝"),
        QCoreApplication::translate("Theme", "森林绿")
    };
}

QString Theme::themeName(ThemeType type)
{
    switch (type) {
        case Dark: return QCoreApplication::translate("Theme", "深色");
        case Light: return QCoreApplication::translate("Theme", "浅色");
        case OceanBlue: return QCoreApplication::translate("Theme", "海洋蓝");
        case ForestGreen: return QCoreApplication::translate("Theme", "森林绿");
        default: return QCoreApplication::translate("Theme", "深色");
    }
}

void Theme::initDarkTheme()
{
    m_primaryColor = "#0E639C";
    m_secondaryColor = "#1177BB";
    m_backgroundColor = "#1E1E1E";
    m_surfaceColor = "#252526";
    m_textColor = "#D4D4D4";
    m_textSecondaryColor = "#888888";
    m_accentColor = "#007ACC";
    m_borderColor = "#3E3E42";
    m_hoverColor = "#2A2D2E";
    m_selectedColor = "#094771";
}

void Theme::initLightTheme()
{
    m_primaryColor = "#2563EB";
    m_secondaryColor = "#1D4ED8";
    m_backgroundColor = "#FFFFFF";
    m_surfaceColor = "#F8FAFC";
    m_textColor = "#1E293B";
    m_textSecondaryColor = "#64748B";
    m_accentColor = "#3B82F6";
    m_borderColor = "#E2E8F0";
    m_hoverColor = "#F1F5F9";
    m_selectedColor = "#DBEAFE";
}

void Theme::initOceanBlueTheme()
{
    m_primaryColor = "#00BCD4";
    m_secondaryColor = "#0097A7";
    m_backgroundColor = "#0D1B2A";
    m_surfaceColor = "#1B2838";
    m_textColor = "#E0F7FA";
    m_textSecondaryColor = "#80CBC4";
    m_accentColor = "#00ACC1";
    m_borderColor = "#263238";
    m_hoverColor = "#1E3A5F";
    m_selectedColor = "#006064";
}

void Theme::initForestGreenTheme()
{
    m_primaryColor = "#8BC34A";
    m_secondaryColor = "#689F38";
    m_backgroundColor = "#1A2F1A";
    m_surfaceColor = "#2E4A2E";
    m_textColor = "#E8F5E9";
    m_textSecondaryColor = "#A5D6A7";
    m_accentColor = "#4CAF50";
    m_borderColor = "#388E3C";
    m_hoverColor = "#3E5E3E";
    m_selectedColor = "#2E7D32";
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

    if (qssPath.isEmpty()) {
        return QString();
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
