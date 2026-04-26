#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "ui/sidebar.h"
#include "ui/theme.h"
#include "core/plugin_manager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUI();
    void setupConnections();
    QWidget* createWelcomePage();

    QSplitter* m_splitter = nullptr;
    Sidebar* m_sidebar = nullptr;
    QStackedWidget* m_stackWidget = nullptr;
    PluginManager* m_pluginManager = nullptr;
    Theme* m_theme = nullptr;
};
