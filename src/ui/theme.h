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
    ThemeType m_currentTheme;
    QString m_accentColor;
};
