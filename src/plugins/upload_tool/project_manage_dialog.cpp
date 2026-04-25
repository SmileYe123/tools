#include "project_manage_dialog.h"

ProjectManageDialog::ProjectManageDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("项目管理"));
    resize(700, 500);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);

    QVBoxLayout* listLayout = new QVBoxLayout();
    listLayout->setSpacing(8);

    QLabel* listLabel = new QLabel(tr("已配置的项目"));
    listLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    listLayout->addWidget(listLabel);

    m_projectsList = new QListWidget();
    m_projectsList->setMinimumWidth(200);
    connect(m_projectsList, &QListWidget::currentRowChanged,
            this, &ProjectManageDialog::onProjectSelectionChanged);
    listLayout->addWidget(m_projectsList);

    m_addBtn = new QPushButton(tr("+ 添加项目"));
    connect(m_addBtn, &QPushButton::clicked, this, &ProjectManageDialog::addProject);
    listLayout->addWidget(m_addBtn);

    m_editBtn = new QPushButton(tr("✏️ 编辑"));
    m_editBtn->setEnabled(false);
    connect(m_editBtn, &QPushButton::clicked, this, &ProjectManageDialog::editProject);
    listLayout->addWidget(m_editBtn);

    m_deleteBtn = new QPushButton(tr("🗑️ 删除"));
    m_deleteBtn->setEnabled(false);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ProjectManageDialog::deleteProject);
    listLayout->addWidget(m_deleteBtn);

    mainLayout->addLayout(listLayout, 1);

    QVBoxLayout* formLayout = new QVBoxLayout();
    formLayout->setSpacing(12);

    QLabel* formLabel = new QLabel(tr("项目详情"));
    formLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    formLayout->addWidget(formLabel);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(10);

    gridLayout->addWidget(new QLabel(tr("项目名称:")), 0, 0);
    m_appNameEdit = new QLineEdit();
    m_appNameEdit->setPlaceholderText(tr("例如: MyApp"));
    gridLayout->addWidget(m_appNameEdit, 0, 1);

    gridLayout->addWidget(new QLabel(tr("服务器目录:")), 1, 0);
    QHBoxLayout* serverDirLayout = new QHBoxLayout();
    m_serverDirEdit = new QLineEdit();
    m_serverDirEdit->setPlaceholderText(tr("选择服务器存储路径"));
    m_serverDirEdit->setReadOnly(true);
    serverDirLayout->addWidget(m_serverDirEdit);
    QPushButton* selectServerBtn = new QPushButton(tr("浏览"));
    connect(selectServerBtn, &QPushButton::clicked, this, &ProjectManageDialog::selectServerDir);
    serverDirLayout->addWidget(selectServerBtn);
    gridLayout->addLayout(serverDirLayout, 1, 1);

    gridLayout->addWidget(new QLabel(tr("服务器 URL:")), 2, 0);
    m_serverUrlEdit = new QLineEdit();
    m_serverUrlEdit->setPlaceholderText(tr("http://192.168.1.100:8080"));
    gridLayout->addWidget(m_serverUrlEdit, 2, 1);

    gridLayout->addWidget(new QLabel(tr("默认 EXE 路径:")), 3, 0);
    QHBoxLayout* exePathLayout = new QHBoxLayout();
    m_defaultExePathEdit = new QLineEdit();
    m_defaultExePathEdit->setPlaceholderText(tr("选择默认 EXE 输出目录"));
    m_defaultExePathEdit->setReadOnly(true);
    exePathLayout->addWidget(m_defaultExePathEdit);
    QPushButton* selectExePathBtn = new QPushButton(tr("浏览"));
    connect(selectExePathBtn, &QPushButton::clicked, this, &ProjectManageDialog::selectDefaultExePath);
    exePathLayout->addWidget(selectExePathBtn);
    gridLayout->addLayout(exePathLayout, 3, 1);

    gridLayout->addWidget(new QLabel(tr("版本前缀:")), 4, 0);
    m_versionPrefixEdit = new QLineEdit();
    m_versionPrefixEdit->setText("V");
    m_versionPrefixEdit->setMaximumWidth(80);
    gridLayout->addWidget(m_versionPrefixEdit, 4, 1);

    formLayout->addLayout(gridLayout);
    formLayout->addStretch();

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton(tr("💾 保存"));
    m_saveBtn->setEnabled(false);
    connect(m_saveBtn, &QPushButton::clicked, this, &ProjectManageDialog::saveProject);
    btnLayout->addWidget(m_saveBtn);

    m_closeBtn = new QPushButton(tr("关闭"));
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_closeBtn);

    formLayout->addLayout(btnLayout);

    mainLayout->addLayout(formLayout, 2);

    loadProjectsList();
}

void ProjectManageDialog::loadProjectsList()
{
    UploadConfig* config = UploadConfig::instance();
    config->load();

    m_projectsList->clear();
    for (int i = 0; i < config->projectCount(); ++i) {
        ProjectConfig pc = config->projectAt(i);
        m_projectsList->addItem(pc.appName);
    }

    if (config->projectCount() > 0) {
        QString lastUsed = config->lastUsedProject();
        int idx = config->findProject(lastUsed);
        if (idx >= 0) {
            m_projectsList->setCurrentRow(idx);
        } else {
            m_projectsList->setCurrentRow(0);
        }
    }
}

void ProjectManageDialog::onProjectSelectionChanged()
{
    int row = m_projectsList->currentRow();
    if (row < 0) {
        clearForm();
        m_editBtn->setEnabled(false);
        m_deleteBtn->setEnabled(false);
        m_saveBtn->setEnabled(false);
        return;
    }

    UploadConfig* config = UploadConfig::instance();
    ProjectConfig pc = config->projectAt(row);
    populateForm(pc);
    m_currentIndex = row;
    m_editBtn->setEnabled(true);
    m_deleteBtn->setEnabled(true);
    m_saveBtn->setEnabled(false);
}

void ProjectManageDialog::populateForm(const ProjectConfig& config)
{
    m_appNameEdit->setText(config.appName);
    m_serverDirEdit->setText(config.serverDir);
    m_serverUrlEdit->setText(config.serverUrl);
    m_defaultExePathEdit->setText(config.defaultExePath);
    m_versionPrefixEdit->setText(config.versionPrefix);
}

void ProjectManageDialog::clearForm()
{
    m_appNameEdit->clear();
    m_serverDirEdit->clear();
    m_serverUrlEdit->clear();
    m_defaultExePathEdit->clear();
    m_versionPrefixEdit->setText("V");
    m_currentIndex = -1;
}

ProjectConfig ProjectManageDialog::getFormConfig() const
{
    ProjectConfig config;
    config.appName = m_appNameEdit->text().trimmed();
    config.serverDir = m_serverDirEdit->text().trimmed();
    config.serverUrl = m_serverUrlEdit->text().trimmed();
    config.defaultExePath = m_defaultExePathEdit->text().trimmed();
    config.versionPrefix = m_versionPrefixEdit->text().trimmed();
    return config;
}

bool ProjectManageDialog::validateConfig()
{
    if (m_appNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("验证失败"), tr("请输入项目名称"));
        return false;
    }
    if (m_serverDirEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("验证失败"), tr("请选择服务器目录"));
        return false;
    }
    if (m_serverUrlEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("验证失败"), tr("请输入服务器 URL"));
        return false;
    }
    return true;
}

void ProjectManageDialog::addProject()
{
    clearForm();
    m_appNameEdit->setFocus();
    m_editBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
    m_saveBtn->setEnabled(true);
    m_currentIndex = -1;
}

void ProjectManageDialog::editProject()
{
    if (m_currentIndex < 0) return;
    m_saveBtn->setEnabled(true);
    m_appNameEdit->setFocus();
}

void ProjectManageDialog::deleteProject()
{
    if (m_currentIndex < 0) return;

    QString appName = m_projectsList->currentItem()->text();
    int ret = QMessageBox::question(this, tr("确认删除"),
        tr("确定要删除项目 '%1' 吗？").arg(appName));
    if (ret != QMessageBox::Yes) return;

    UploadConfig* config = UploadConfig::instance();
    config->removeProject(m_currentIndex);
    config->save();

    loadProjectsList();
    clearForm();
    m_saveBtn->setEnabled(false);
    emit configUpdated();
}

void ProjectManageDialog::saveProject()
{
    if (!validateConfig()) return;

    ProjectConfig newConfig = getFormConfig();
    UploadConfig* config = UploadConfig::instance();

    if (m_currentIndex < 0) {
        if (config->findProject(newConfig.appName) >= 0) {
            QMessageBox::warning(this, tr("保存失败"), tr("项目 '%1' 已存在").arg(newConfig.appName));
            return;
        }
        config->addProject(newConfig);
        QMessageBox::information(this, tr("成功"), tr("项目 '%1' 已添加").arg(newConfig.appName));
    } else {
        config->updateProject(m_currentIndex, newConfig);
        QMessageBox::information(this, tr("成功"), tr("项目 '%1' 已更新").arg(newConfig.appName));
    }

    config->save();
    loadProjectsList();
    m_saveBtn->setEnabled(false);
    emit configUpdated();
}

void ProjectManageDialog::selectServerDir()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择服务器目录"), "");
    if (!dirPath.isEmpty()) {
        m_serverDirEdit->setText(dirPath);
        m_saveBtn->setEnabled(m_currentIndex >= 0 || !m_appNameEdit->text().isEmpty());
    }
}

void ProjectManageDialog::selectDefaultExePath()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("选择默认 EXE 目录"), "");
    if (!dirPath.isEmpty()) {
        m_defaultExePathEdit->setText(dirPath);
        m_saveBtn->setEnabled(m_currentIndex >= 0 || !m_appNameEdit->text().isEmpty());
    }
}
