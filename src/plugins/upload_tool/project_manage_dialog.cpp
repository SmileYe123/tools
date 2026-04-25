#include "project_manage_dialog.h"

ProjectManageDialog::ProjectManageDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("项目管理"));
    resize(700, 550);

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

    int row = 0;
    gridLayout->addWidget(new QLabel(tr("项目名称:")), row, 0);
    m_appNameEdit = new QLineEdit();
    m_appNameEdit->setPlaceholderText(tr("例如: MyApp"));
    gridLayout->addWidget(m_appNameEdit, row, 1);

    row++;
    gridLayout->addWidget(new QLabel(tr("发布模式:")), row, 0);
    m_publishModeCombo = new QComboBox();
    m_publishModeCombo->addItem(tr("📁 本地服务器（复制到本地目录）"), static_cast<int>(PublishMode::LocalServer));
    m_publishModeCombo->addItem(tr("🌐 远程服务器（HTTP 上传）"), static_cast<int>(PublishMode::RemoteServer));
    m_publishModeCombo->addItem(tr("🖥️ 自身作为服务器（启动内置 HTTP 服务）"), static_cast<int>(PublishMode::SelfAsServer));
    connect(m_publishModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProjectManageDialog::onPublishModeChanged);
    gridLayout->addWidget(m_publishModeCombo, row, 1);

    row++;
    m_serverDirLabel = new QLabel(tr("服务器目录:"));
    gridLayout->addWidget(m_serverDirLabel, row, 0);
    QHBoxLayout* serverDirLayout = new QHBoxLayout();
    m_serverDirEdit = new QLineEdit();
    m_serverDirEdit->setPlaceholderText(tr("选择服务器存储路径"));
    m_serverDirEdit->setReadOnly(true);
    serverDirLayout->addWidget(m_serverDirEdit);
    QPushButton* selectServerBtn = new QPushButton(tr("浏览"));
    connect(selectServerBtn, &QPushButton::clicked, this, &ProjectManageDialog::selectServerDir);
    serverDirLayout->addWidget(selectServerBtn);
    gridLayout->addLayout(serverDirLayout, row, 1);

    row++;
    m_serverUrlLabel = new QLabel(tr("服务器 URL:"));
    gridLayout->addWidget(m_serverUrlLabel, row, 0);
    m_serverUrlEdit = new QLineEdit();
    m_serverUrlEdit->setPlaceholderText(tr("http://192.168.1.100:8080"));
    gridLayout->addWidget(m_serverUrlEdit, row, 1);

    row++;
    gridLayout->addWidget(new QLabel(tr("内置服务端口:")), row, 0);
    m_selfServerPortSpin = new QSpinBox();
    m_selfServerPortSpin->setRange(1, 65535);
    m_selfServerPortSpin->setValue(8080);
    m_selfServerPortSpin->setEnabled(false);
    gridLayout->addWidget(m_selfServerPortSpin, row, 1);

    row++;
    gridLayout->addWidget(new QLabel(tr("默认 EXE 路径:")), row, 0);
    QHBoxLayout* exePathLayout = new QHBoxLayout();
    m_defaultExePathEdit = new QLineEdit();
    m_defaultExePathEdit->setPlaceholderText(tr("选择默认 EXE 输出目录"));
    m_defaultExePathEdit->setReadOnly(true);
    exePathLayout->addWidget(m_defaultExePathEdit);
    QPushButton* selectExePathBtn = new QPushButton(tr("浏览"));
    connect(selectExePathBtn, &QPushButton::clicked, this, &ProjectManageDialog::selectDefaultExePath);
    exePathLayout->addWidget(selectExePathBtn);
    gridLayout->addLayout(exePathLayout, row, 1);

    row++;
    gridLayout->addWidget(new QLabel(tr("版本前缀:")), row, 0);
    m_versionPrefixEdit = new QLineEdit();
    m_versionPrefixEdit->setText("V");
    m_versionPrefixEdit->setMaximumWidth(80);
    gridLayout->addWidget(m_versionPrefixEdit, row, 1);

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
    m_publishModeCombo->setCurrentIndex(static_cast<int>(config.publishMode));
    m_selfServerPortSpin->setValue(config.selfServerPort);
    onPublishModeChanged(static_cast<int>(config.publishMode));
}

void ProjectManageDialog::clearForm()
{
    m_appNameEdit->clear();
    m_serverDirEdit->clear();
    m_serverUrlEdit->clear();
    m_defaultExePathEdit->clear();
    m_versionPrefixEdit->setText("V");
    m_publishModeCombo->setCurrentIndex(0);
    m_selfServerPortSpin->setValue(8080);
    m_currentIndex = -1;
    onPublishModeChanged(0);
}

ProjectConfig ProjectManageDialog::getFormConfig() const
{
    ProjectConfig config;
    config.appName = m_appNameEdit->text().trimmed();
    config.serverDir = m_serverDirEdit->text().trimmed();
    config.serverUrl = m_serverUrlEdit->text().trimmed();
    config.defaultExePath = m_defaultExePathEdit->text().trimmed();
    config.versionPrefix = m_versionPrefixEdit->text().trimmed();
    config.publishMode = static_cast<PublishMode>(m_publishModeCombo->currentIndex());
    config.selfServerPort = m_selfServerPortSpin->value();
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

void ProjectManageDialog::onPublishModeChanged(int index)
{
    PublishMode mode = static_cast<PublishMode>(index);
    
    switch (mode) {
    case PublishMode::LocalServer:
        // 本地服务器模式：需要服务器目录，不需要 URL
        m_serverDirLabel->setText(tr("服务器目录:"));
        m_serverUrlLabel->setText(tr("服务器 URL:"));
        m_serverDirEdit->setEnabled(true);
        m_serverDirEdit->setReadOnly(true);
        m_serverUrlEdit->setEnabled(true);
        m_serverUrlEdit->setReadOnly(false);
        m_selfServerPortSpin->setEnabled(false);
        break;
        
    case PublishMode::RemoteServer:
        // 远程服务器模式：需要服务器 URL，不需要目录
        m_serverDirLabel->setText(tr("本地缓存目录:"));
        m_serverUrlLabel->setText(tr("远程服务器 URL:"));
        m_serverDirEdit->setEnabled(true);
        m_serverDirEdit->setReadOnly(true);
        m_serverUrlEdit->setEnabled(true);
        m_serverUrlEdit->setReadOnly(false);
        m_selfServerPortSpin->setEnabled(false);
        break;
        
    case PublishMode::SelfAsServer:
        // 自身作为服务器模式：需要端口，使用本地目录
        m_serverDirLabel->setText(tr("服务根目录:"));
        m_serverUrlLabel->setText(tr("访问 URL:"));
        m_serverDirEdit->setEnabled(true);
        m_serverDirEdit->setReadOnly(true);
        m_serverUrlEdit->setEnabled(true);
        m_serverUrlEdit->setReadOnly(true);
        m_selfServerPortSpin->setEnabled(true);
        // 自动生成 URL
        m_serverUrlEdit->setText(QString("http://localhost:%1").arg(m_selfServerPortSpin->value()));
        break;
    }
}
