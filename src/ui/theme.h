#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class Theme : public QObject
{
    Q_OBJECT

public:
    enum ThemeType {
        Light,
        Dark,
        OceanBlue,
        ForestGreen
    };
    Q_ENUM(ThemeType)

    explicit Theme(QObject* parent = nullptr);
    explicit Theme(ThemeType type, QObject* parent = nullptr);

    void setTheme(ThemeType type);
    ThemeType currentTheme() const;
    QString getAccentColor() const;
    QString getStyleSheet() const;

    static QStringList availableThemes();
    static QString themeName(ThemeType type);

private:
    void initDarkTheme();
    void initLightTheme();
    void initOceanBlueTheme();
    void initForestGreenTheme();

    ThemeType m_currentTheme;
    QString m_primaryColor;
    QString m_secondaryColor;
    QString m_backgroundColor;
    QString m_surfaceColor;
    QString m_textColor;
    QString m_textSecondaryColor;
    QString m_accentColor;
    QString m_borderColor;
    QString m_hoverColor;
    QString m_selectedColor;
};
