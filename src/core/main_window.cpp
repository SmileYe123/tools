#include "main_window.h"
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    m_theme = new Theme(this);
    m_pluginManager = new PluginManager(this);
    m_pluginManager->loadPlugins();

    setupUI();
    setupConnections();

    setWindowTitle(tr("MultiTool - 多功能工具箱"));
    resize(1200, 800);

    QScreen* screen = QApplication::primaryScreen();
    QRect geometry = screen->availableGeometry();
    move(geometry.center() - rect().center());
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    m_splitter = new QSplitter(Qt::Horizontal, this);

    m_sidebar = new Sidebar(this);
    m_sidebar->setFixedWidth(250);

    m_stackWidget = new QStackedWidget(this);

    QWidget* welcomeWidget = new QWidget(m_stackWidget);
    QVBoxLayout* welcomeLayout = new QVBoxLayout(welcomeWidget);
    welcomeLayout->setAlignment(Qt::AlignCenter);
    m_welcomeLabel = new QLabel(tr("欢迎使用 MultiTool 多功能工具箱\n\n请选择左侧工具开始使用"), welcomeWidget);
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setWordWrap(true);
    QFont font = m_welcomeLabel->font();
    font.setPointSize(14);
    m_welcomeLabel->setFont(font);
    welcomeLayout->addWidget(m_welcomeLabel);
    m_stackWidget->addWidget(welcomeWidget);

    for (BasePlugin* plugin : m_pluginManager->plugins()) {
        QWidget* widget = plugin->createWidget(m_stackWidget);
        m_stackWidget->addWidget(widget);
    }

    m_splitter->addWidget(m_sidebar);
    m_splitter->addWidget(m_stackWidget);
    m_splitter->setStretchFactor(1, 1);

    setCentralWidget(m_splitter);

    m_sidebar->setupToolButtons(m_pluginManager->plugins());

    QStatusBar* statusBar = new QStatusBar(this);
    statusBar->showMessage(tr("就绪"));
    setStatusBar(statusBar);
}

void MainWindow::setupConnections()
{
    connect(m_sidebar, &Sidebar::toolSelected, this, [this](int index) {
        m_stackWidget->setCurrentIndex(index + 1);
    });
}
