#include "sidebar.h"
#include <QButtonGroup>

Sidebar::Sidebar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("sidebar");

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    QWidget* headerWidget = new QWidget(this);
    headerWidget->setObjectName("sidebar_header");
    headerWidget->setFixedHeight(80);

    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    QLabel* titleLabel = new QLabel(tr("多功能工具箱"), headerWidget);
    titleLabel->setObjectName("sidebar_title");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    headerLayout->addWidget(titleLabel);

    m_searchEdit = new QLineEdit(headerWidget);
    m_searchEdit->setObjectName("sidebar_search");
    m_searchEdit->setPlaceholderText(tr("搜索工具..."));
    headerLayout->addWidget(m_searchEdit);

    m_mainLayout->addWidget(headerWidget);

    QFrame* line = new QFrame(this);
    line->setObjectName("sidebar_line");
    line->setFixedHeight(1);
    m_mainLayout->addWidget(line);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("sidebar_scroll");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_scrollWidget = new QWidget(m_scrollArea);
    m_scrollWidget->setObjectName("sidebar_scroll_widget");

    m_buttonsLayout = new QVBoxLayout(m_scrollWidget);
    m_buttonsLayout->setContentsMargins(8, 8, 8, 8);
    m_buttonsLayout->setSpacing(4);
    m_buttonsLayout->addStretch();

    m_scrollArea->setWidget(m_scrollWidget);
    m_mainLayout->addWidget(m_scrollArea);
}

void Sidebar::setupToolButtons(const QList<BasePlugin*>& plugins)
{
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);

    for (int i = 0; i < plugins.size(); ++i) {
        BasePlugin* plugin = plugins[i];
        QToolButton* button = new QToolButton(m_scrollWidget);
        button->setText(plugin->icon() + "  " + plugin->name());
        button->setCheckable(true);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setFixedHeight(48);

        m_buttonsLayout->insertWidget(m_buttonsLayout->count() - 1, button);
        m_toolButtons.append(button);

        connect(button, &QToolButton::clicked, this, [this, i]() {
            emit toolSelected(i);
        });

        buttonGroup->addButton(button);
    }
}
