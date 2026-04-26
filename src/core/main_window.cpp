#include "main_window.h"
#include <QApplication>
#include <QScreen>
#include <QGridLayout>
#include <QMouseEvent>

class ClickableCard : public QWidget
{
    Q_OBJECT
public:
    ClickableCard(QWidget* parent = nullptr) : QWidget(parent) {}
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) emit clicked();
        QWidget::mousePressEvent(event);
    }
};

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
    if (screen) {
        QRect geometry = screen->availableGeometry();
        move(geometry.center() - rect().center());
    }
}

MainWindow::~MainWindow()
{
}

QWidget* MainWindow::createWelcomePage()
{
    QWidget* page = new QWidget(m_stackWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(page);
    mainLayout->setContentsMargins(60, 40, 60, 40);
    mainLayout->setSpacing(40);

    QLabel* titleLabel = new QLabel(tr("欢迎使用 MultiTool"), page);
    titleLabel->setObjectName("welcome_title");
    titleLabel->setAlignment(Qt::AlignLeft);

    QLabel* subtitleLabel = new QLabel(tr("选择左侧工具开始使用，或点击快捷卡片快速访问"), page);
    subtitleLabel->setObjectName("welcome_subtitle");
    subtitleLabel->setAlignment(Qt::AlignLeft);

    QVBoxLayout* headerLayout = new QVBoxLayout();
    headerLayout->setSpacing(12);
    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);

    mainLayout->addLayout(headerLayout);

    QGridLayout* cardsLayout = new QGridLayout();
    cardsLayout->setSpacing(20);
    cardsLayout->setContentsMargins(0, 0, 0, 0);

    QList<QPair<QString, QString>> cards = {
        {tr("JSON格式化"), tr("美化和验证 JSON 数据")},
        {tr("时间戳工具"), tr("Unix 时间戳与日期互转")},
        {tr("颜色选择"), tr("拾取和转换颜色值")},
        {tr("文本工具"), tr("文本转换和统计")},
        {tr("哈希计算"), tr("计算文件哈希值")},
        {tr("正则工具"), tr("测试和调试正则表达式")},
    };

    // 预构建插件名称到索引的映射，避免重复搜索
    QMap<QString, int> pluginIndexMap;
    const auto& plugins = m_pluginManager->plugins();
    for (int i = 0; i < plugins.size(); ++i) {
        pluginIndexMap[plugins[i]->name()] = i;
    }

    int row = 0, col = 0;
    for (const auto& card : cards) {
        ClickableCard* cardWidget = new ClickableCard(page);
        cardWidget->setObjectName("quick_card");
        cardWidget->setMinimumHeight(120);
        cardWidget->setCursor(Qt::PointingHandCursor);

        QVBoxLayout* layout = new QVBoxLayout(cardWidget);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(12);

        QLabel* cardTitleLabel = new QLabel(card.first, cardWidget);
        cardTitleLabel->setObjectName("card_title");
        QFont titleFont = cardTitleLabel->font();
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        cardTitleLabel->setFont(titleFont);

        QLabel* descLabel = new QLabel(card.second, cardWidget);
        descLabel->setObjectName("card_desc");
        descLabel->setWordWrap(true);

        layout->addWidget(cardTitleLabel);
        layout->addWidget(descLabel);

        int pluginIndex = pluginIndexMap.value(card.first, -1);
        if (pluginIndex >= 0) {
            connect(cardWidget, &ClickableCard::clicked, this, [this, pluginIndex]() {
                m_stackWidget->setCurrentIndex(pluginIndex + 1);
                m_sidebar->selectTool(pluginIndex);
            });
        }

        cardsLayout->addWidget(cardWidget, row, col);

        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }

    mainLayout->addLayout(cardsLayout);
    mainLayout->addStretch();

    return page;
}

void MainWindow::setupUI()
{
    m_splitter = new QSplitter(Qt::Horizontal, this);

    m_sidebar = new Sidebar(this);
    m_sidebar->setFixedWidth(260);

    m_stackWidget = new QStackedWidget(this);

    QWidget* welcomeWidget = createWelcomePage();
    welcomeWidget->setObjectName("welcome_page");
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

#include "main_window.moc"
