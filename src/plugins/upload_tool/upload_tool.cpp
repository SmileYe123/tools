#include "upload_tool.h"

QWidget* UploadToolPlugin::createWidget(QWidget* parent)
{
    m_widget = new QWidget(parent);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_widget);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    UploadConfig::instance()->load();

    QGroupBox* selectGroup = new QGroupBox(tr("项目选择"));
    QHBoxLayout* selectLayout = new QHBoxLayout(selectGroup);
    selectLayout->setSpacing(10);

    selectLayout->addWidget(new QLabel(tr("选择项目:")));
    m_projectCombo = new QComboBox();
    for (int i = 0; i < UploadConfig::instance()->projectCount(); ++i) {
        ProjectConfig pc = UploadConfig::instance()->projectAt(i);
        m_projectCombo->addItem(pc.appName);
    }
    QString lastUsed = UploadConfig::instance()->lastUsedProject();
    int lastIdx = UploadConfig::instance()->findProject(lastUsed);
    if (lastIdx >= 0) {
        m_projectCombo->setCurrentIndex(lastIdx);
    }
    connect(m_projectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UploadToolPlugin::onProjectSelected);
    selectLayout->addWidget(m_projectCombo, 1);

    m_manageProjectsBtn = new QPushButton(tr("⚙️ 管理项目"));
    connect(m_manageProjectsBtn, &QPushButton::clicked, this, &UploadToolPlugin::openProjectManager);
    selectLayout->addWidget(m_manageProjectsBtn);

    mainLayout->addWidget(selectGroup);

    QGroupBox* configGroup = new QGroupBox(tr("发布配置"));
    QGridLayout* configLayout = new QGridLayout(configGroup);
    configLayout->setSpacing(10);
    configLayout->setContentsMargins(14, 18, 14, 14);

    configLayout->addWidget(new QLabel(tr("项目名称:")), 0, 0);
    m_appNameEdit = new QLineEdit();
    m_appNameEdit->setReadOnly(true);
    configLayout->addWidget(m_appNameEdit, 0, 1);

    configLayout->addWidget(new QLabel(tr("版本号:")), 1, 0);
    m_versionEdit = new QLineEdit();
    m_versionEdit->setPlaceholderText(tr("例如: V1.0.0 (自动从文件名提取)"));
    configLayout->addWidget(m_versionEdit, 1, 1);

    QHBoxLayout* exeLayout = new QHBoxLayout();
    m_exePathEdit = new QLineEdit();
    m_exePathEdit->setPlaceholderText(tr("选择要发布的 EXE 文件"));
    m_exePathEdit->setReadOnly(true);
    exeLayout->addWidget(m_exePathEdit);
    QPushButton* selectExeBtn = new QPushButton(tr("浏览"));
    connect(selectExeBtn, &QPushButton::clicked, this, &UploadToolPlugin::selectExeFile);
    exeLayout->addWidget(selectExeBtn);
    configLayout->addWidget(new QLabel(tr("EXE 文件:")), 2, 0);
    configLayout->addLayout(exeLayout, 2, 1);

    configLayout->addWidget(new QLabel(tr("服务器目录:")), 3, 0);
    m_serverDirEdit = new QLineEdit();
    m_serverDirEdit->setReadOnly(true);
    configLayout->addWidget(m_serverDirEdit, 3, 1);

    configLayout->addWidget(new QLabel(tr("服务器 URL:")), 4, 0);
    m_serverUrlEdit = new QLineEdit();
    m_serverUrlEdit->setReadOnly(true);
    configLayout->addWidget(m_serverUrlEdit, 4, 1);

    mainLayout->addWidget(configGroup);

    QGroupBox* notesGroup = new QGroupBox(tr("更新日志"));
    QVBoxLayout* notesLayout = new QVBoxLayout(notesGroup);
    m_releaseNotesEdit = new QTextEdit();
    m_releaseNotesEdit->setPlaceholderText(tr("输入更新说明（支持 Markdown 格式）\n例如：\n## V1.0.0\n\n### 新功能\n- 新增 XXX 功能"));
    m_releaseNotesEdit->setMaximumHeight(100);
    notesLayout->addWidget(m_releaseNotesEdit);
    mainLayout->addWidget(notesGroup);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_publishBtn = new QPushButton(tr("🚀 发布到服务器"));
    m_publishBtn->setObjectName("primaryBtn");
    connect(m_publishBtn, &QPushButton::clicked, this, &UploadToolPlugin::publishUpdate);
    btnLayout->addWidget(m_publishBtn);

    m_clearBtn = new QPushButton(tr("🗑️ 清空"));
    connect(m_clearBtn, &QPushButton::clicked, this, &UploadToolPlugin::clearForm);
    btnLayout->addWidget(m_clearBtn);

    mainLayout->addLayout(btnLayout);

    QGroupBox* outputGroup = new QGroupBox(tr("发布日志"));
    QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
    m_outputEdit = new QTextEdit();
    m_outputEdit->setReadOnly(true);
    m_outputEdit->setPlaceholderText(tr("发布过程日志..."));
    outputLayout->addWidget(m_outputEdit);
    mainLayout->addWidget(outputGroup);

    if (m_projectCombo->count() > 0) {
        onProjectSelected(m_projectCombo->currentIndex());
    }

    return m_widget;
}

void UploadToolPlugin::onProjectSelected(int index)
{
    m_isLoadingConfig = true;
    if (index < 0 || index >= UploadConfig::instance()->projectCount()) {
        m_appNameEdit->clear();
        m_serverDirEdit->clear();
        m_serverUrlEdit->clear();
        m_isLoadingConfig = false;
        return;
    }

    ProjectConfig pc = UploadConfig::instance()->projectAt(index);
    m_appNameEdit->setText(pc.appName);
    m_serverDirEdit->setText(pc.serverDir);
    m_serverUrlEdit->setText(pc.serverUrl);

    if (!m_exePathEdit->text().isEmpty()) {
        autoExtractVersion(m_exePathEdit->text());
    }

    UploadConfig::instance()->setLastUsedProject(pc.appName);
    UploadConfig::instance()->save();
    m_isLoadingConfig = false;
}

void UploadToolPlugin::selectExeFile()
{
    QString startDir;
    if (UploadConfig::instance()->projectCount() > 0) {
        int idx = m_projectCombo->currentIndex();
        if (idx >= 0) {
            ProjectConfig pc = UploadConfig::instance()->projectAt(idx);
            if (!pc.defaultExePath.isEmpty()) {
                startDir = pc.defaultExePath;
            }
        }
    }

    QString filePath = QFileDialog::getOpenFileName(nullptr,
        tr("选择 EXE 文件"), startDir, tr("可执行文件 (*.exe);;所有文件 (*)"));
    if (!filePath.isEmpty()) {
        m_exePathEdit->setText(filePath);
        autoExtractVersion(filePath);
    }
}

void UploadToolPlugin::openProjectManager()
{
    ProjectManageDialog dialog(m_widget);
    connect(&dialog, &ProjectManageDialog::configUpdated, this, [this]() {
        m_projectCombo->clear();
        for (int i = 0; i < UploadConfig::instance()->projectCount(); ++i) {
            ProjectConfig pc = UploadConfig::instance()->projectAt(i);
            m_projectCombo->addItem(pc.appName);
        }
        if (m_projectCombo->count() > 0) {
            QString lastUsed = UploadConfig::instance()->lastUsedProject();
            int idx = UploadConfig::instance()->findProject(lastUsed);
            if (idx >= 0) {
                m_projectCombo->setCurrentIndex(idx);
            } else {
                m_projectCombo->setCurrentIndex(0);
            }
            onProjectSelected(m_projectCombo->currentIndex());
        }
    });
    dialog.exec();
}

void UploadToolPlugin::autoExtractVersion(const QString& exePath)
{
    if (m_isLoadingConfig) return;

    QFileInfo fileInfo(exePath);
    QString fileName = fileInfo.baseName();
    QString versionPrefix = "V";

    int idx = m_projectCombo->currentIndex();
    if (idx >= 0) {
        ProjectConfig pc = UploadConfig::instance()->projectAt(idx);
        if (!pc.versionPrefix.isEmpty()) {
            versionPrefix = pc.versionPrefix;
        }
    }

    QRegularExpression versionRegex("(V\\d+\\.\\d+\\.\\d+(?:_SP\\d+)?)");
    QRegularExpressionMatch match = versionRegex.match(fileName);
    if (match.hasMatch()) {
        m_versionEdit->setText(match.captured(1));
    } else {
        QRegularExpression legacyRegex("(\\d+\\.\\d+\\.\\d+(?:-[a-zA-Z0-9.]+)?)");
        QRegularExpressionMatch legacyMatch = legacyRegex.match(fileName);
        if (legacyMatch.hasMatch()) {
            m_versionEdit->setText(versionPrefix + legacyMatch.captured(1));
        }
    }
}

void UploadToolPlugin::publishUpdate()
{
    m_outputEdit->clear();
    QString appName = m_appNameEdit->text().trimmed();
    QString version = m_versionEdit->text().trimmed();
    QString exePath = m_exePathEdit->text().trimmed();
    QString serverDir = m_serverDirEdit->text().trimmed();
    QString serverUrl = m_serverUrlEdit->text().trimmed();
    QString releaseNotes = m_releaseNotesEdit->toPlainText().trimmed();

    if (appName.isEmpty()) {
        QMessageBox::warning(nullptr, tr("参数错误"), tr("请选择或配置项目"));
        return;
    }
    if (version.isEmpty()) {
        QMessageBox::warning(nullptr, tr("参数错误"), tr("请输入版本号"));
        return;
    }
    if (exePath.isEmpty() || !QFile::exists(exePath)) {
        QMessageBox::warning(nullptr, tr("参数错误"), tr("请选择有效的 EXE 文件"));
        return;
    }
    if (serverDir.isEmpty()) {
        QMessageBox::warning(nullptr, tr("参数错误"), tr("请配置服务器目录"));
        return;
    }

    m_outputEdit->append(tr("[1/4] 计算 SHA-256 哈希..."));
    QString hash = calculateSHA256(exePath);
    if (hash.isEmpty()) {
        m_outputEdit->append(tr("  ✗ 计算哈希失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法计算 EXE 文件的 SHA-256 哈希"));
        return;
    }
    m_outputEdit->append(tr("  ✓ SHA256: %1").arg(hash));

    QString packagesDir = QDir(serverDir).filePath(appName + "/packages");
    QFileInfo exeFileInfo(exePath);
    QString exeFileName = exeFileInfo.fileName();
    QString destExePath = QDir(packagesDir).filePath(exeFileName);

    m_outputEdit->append(tr("\n[2/4] 复制 EXE 到服务器..."));
    m_outputEdit->append(tr("  目标: %1").arg(destExePath));

    if (!QDir().mkpath(packagesDir)) {
        m_outputEdit->append(tr("  ✗ 创建目录失败: %1").arg(packagesDir));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法创建服务器目录: %1").arg(packagesDir));
        return;
    }
    m_outputEdit->append(tr("  ✓ 已创建目录: %1").arg(packagesDir));

    if (!copyExeToServer(exePath, destExePath)) {
        m_outputEdit->append(tr("  ✗ 复制文件失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法复制 EXE 文件到服务器"));
        return;
    }
    m_outputEdit->append(tr("  ✓ 已复制到: %1").arg(destExePath));

    qint64 fileSize = exeFileInfo.size();
    QString versionJsonPath = QDir(serverDir).filePath(appName + "/version.json");

    m_outputEdit->append(tr("\n[3/4] 更新 version.json..."));
    m_outputEdit->append(tr("  文件: %1").arg(versionJsonPath));

    if (!updateVersionJson(versionJsonPath, appName, version, exeFileName, fileSize,
                          "sha256:" + hash, serverUrl, releaseNotes)) {
        m_outputEdit->append(tr("  ✗ 更新 version.json 失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法更新 version.json 文件"));
        return;
    }
    m_outputEdit->append(tr("  ✓ version.json 已更新"));

    m_outputEdit->append(tr("\n[4/4] 验证发布结果..."));
    if (QFile::exists(destExePath) && QFile::exists(versionJsonPath)) {
        m_outputEdit->append(tr("  ✓ 验证通过！文件已正确上传"));
        m_outputEdit->append(tr("  下载 URL: %1/%2/packages/%3").arg(serverUrl, appName, exeFileName));
        m_outputEdit->append(tr("\n✅ 发布完成！"));
        QMessageBox::information(nullptr, tr("发布成功"),
            tr("版本 %1 已成功发布到服务器\n\n下载 URL:\n%2").arg(version, serverUrl + "/" + appName + "/packages/" + exeFileName));
    } else {
        m_outputEdit->append(tr("  ✗ 验证失败：文件不存在"));
        QMessageBox::warning(nullptr, tr("发布警告"), tr("发布可能未成功完成，请检查服务器目录"));
    }
}

void UploadToolPlugin::clearForm()
{
    m_versionEdit->clear();
    m_exePathEdit->clear();
    m_releaseNotesEdit->clear();
    m_outputEdit->clear();
}

QString UploadToolPlugin::calculateSHA256(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    qint64 fileSize = file.size();
    QCryptographicHash hash(QCryptographicHash::Sha256);

    QProgressDialog progress(tr("正在计算 SHA-256..."), tr("取消"), 0, 100, nullptr);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    qint64 bytesRead = 0;
    const qint64 chunkSize = 8192;
    int lastPct = 0;
    while (!file.atEnd()) {
        if (progress.wasCanceled()) {
            file.close();
            return QString();
        }
        QByteArray data = file.read(chunkSize);
        if (data.isEmpty()) break;
        hash.addData(data);
        bytesRead += data.size();

        int pct = static_cast<int>((bytesRead * 100) / fileSize);
        if (pct != lastPct) {
            progress.setValue(pct);
            lastPct = pct;
        }
    }
    file.close();

    progress.setValue(100);

    return hash.result().toHex();
}

bool UploadToolPlugin::copyExeToServer(const QString& srcPath, const QString& destPath)
{
    if (QFile::exists(destPath)) {
        QFile::remove(destPath);
    }
    return QFile::copy(srcPath, destPath);
}

bool UploadToolPlugin::updateVersionJson(const QString& jsonPath, const QString& appName,
                                         const QString& version, const QString& filename,
                                         qint64 fileSize, const QString& hash, const QString& serverUrl,
                                         const QString& releaseNotes)
{
    QJsonObject jsonObj;

    if (QFile::exists(jsonPath)) {
        QFile jsonFile(jsonPath);
        if (jsonFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(jsonFile.readAll());
            jsonFile.close();
            if (doc.isObject()) {
                jsonObj = doc.object();
            }
        }
    }

    jsonObj["app_name"] = appName;
    jsonObj["latest_version"] = version;
    jsonObj["release_date"] = QDate::currentDate().toString("yyyy-MM-dd");

    if (!releaseNotes.isEmpty()) {
        jsonObj["release_notes"] = releaseNotes;
    }

    QJsonObject exeObj;
    exeObj["filename"] = filename;
    exeObj["url"] = QString("%1/%2/packages/%3").arg(serverUrl, appName, filename);
    exeObj["size"] = fileSize;
    exeObj["hash"] = hash;
    jsonObj["exe"] = exeObj;

    QJsonDocument newDoc(jsonObj);
    QFile jsonFile(jsonPath);
    if (!jsonFile.open(QIODevice::WriteOnly)) {
        return false;
    }
    jsonFile.write(newDoc.toJson(QJsonDocument::Indented));
    jsonFile.close();

    return true;
}
