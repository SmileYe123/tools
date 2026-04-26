#include "upload_tool.h"

// SimpleHttpServer 实现
SimpleHttpServer::SimpleHttpServer(QObject* parent)
    : QObject(parent)
{
}

SimpleHttpServer::~SimpleHttpServer()
{
    stop();
}

bool SimpleHttpServer::start(const QString& rootDir, quint16 port)
{
    m_rootDir = rootDir;
    m_server = new QTcpServer(this);
    
    if (!m_server->listen(QHostAddress::Any, port)) {
        emit error(tr("无法监听端口 %1: %2").arg(port).arg(m_server->errorString()));
        delete m_server;
        m_server = nullptr;
        return false;
    }
    
    connect(m_server, &QTcpServer::newConnection, this, &SimpleHttpServer::onNewConnection);
    return true;
}

void SimpleHttpServer::stop()
{
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
    }
    // 清理所有客户端连接
    for (auto it = m_buffers.begin(); it != m_buffers.end(); ++it) {
        if (it.key()) {
            it.key()->disconnectFromHost();
        }
    }
    m_buffers.clear();
}

void SimpleHttpServer::onNewConnection()
{
    QTcpSocket* socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &SimpleHttpServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &SimpleHttpServer::onClientDisconnected);
    m_buffers[socket] = QByteArray();
}

void SimpleHttpServer::onClientDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        m_buffers.remove(socket);
        socket->deleteLater();
    }
}

void SimpleHttpServer::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    m_buffers[socket] += socket->readAll();
    
    // 检查是否收到完整的 HTTP 请求
    if (m_buffers[socket].contains("\r\n\r\n")) {
        QByteArray data = m_buffers[socket];
        m_buffers.remove(socket);
        
        // 解析请求行
        int lineEnd = data.indexOf("\r\n");
        QString requestLine = QString::fromUtf8(data.left(lineEnd));
        QStringList parts = requestLine.split(' ');
        
        if (parts.size() >= 2) {
            QString method = parts[0];
            QString path = parts[1];
            handleRequest(socket, method, path);
        } else {
            sendResponse(socket, 400, "Bad Request", QByteArray(), "text/plain");
        }
    }
}

void SimpleHttpServer::handleRequest(QTcpSocket* socket, const QString& method, const QString& path)
{
    Q_UNUSED(method)  // 当前只支持 GET 请求
    
    // URL 解码
    QString decodedPath = QUrl::fromPercentEncoding(path.toUtf8());
    
    // 路由处理
    if (decodedPath == "/health") {
        QJsonObject resp{
            {"status", "ok"},
            {"version", "1.0.0"},
            {"uptime", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
        sendResponse(socket, 200, "OK", QJsonDocument(resp).toJson(), "application/json");
    }
    else if (decodedPath == "/projects") {
        QJsonArray projects;
        QDir rootDir(m_rootDir);
        for (const QFileInfo& info : rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            QString versionFile = info.absoluteFilePath() + "/version.json";
            if (QFile::exists(versionFile)) {
                QFile vf(versionFile);
                if (vf.open(QIODevice::ReadOnly)) {
                    QJsonDocument doc = QJsonDocument::fromJson(vf.readAll());
                    vf.close();
                    if (doc.isObject()) {
                        QJsonObject obj = doc.object();
                        obj["project_key"] = info.fileName();
                        projects.append(obj);
                    }
                }
            }
        }
        sendResponse(socket, 200, "OK", QJsonDocument(QJsonObject{{"projects", projects}}).toJson(), "application/json");
    }
    else if (decodedPath.endsWith("/version.json")) {
        // 提取 project_key
        QStringList parts = decodedPath.split('/', QString::SkipEmptyParts);
        if (parts.size() >= 2) {
            QString projectKey = parts[0];
            if (isPathSafe(projectKey)) {
                QString filePath = m_rootDir + "/" + projectKey + "/version.json";
                sendFile(socket, filePath, "application/json");
            } else {
                sendResponse(socket, 403, "Forbidden", QByteArray(), "text/plain");
            }
        } else {
            sendResponse(socket, 404, "Not Found", QByteArray(), "text/plain");
        }
    }
    else if (decodedPath.contains("/packages/")) {
        // 提取 project_key 和 filename
        QStringList parts = decodedPath.split('/', QString::SkipEmptyParts);
        if (parts.size() >= 3) {
            QString projectKey = parts[0];
            QString filename = parts[2];
            if (isPathSafe(projectKey) && isPathSafe(filename)) {
                QString filePath = m_rootDir + "/" + projectKey + "/packages/" + filename;
                sendFile(socket, filePath, "application/octet-stream");
            } else {
                sendResponse(socket, 403, "Forbidden", QByteArray(), "text/plain");
            }
        } else {
            sendResponse(socket, 404, "Not Found", QByteArray(), "text/plain");
        }
    }
    else {
        sendResponse(socket, 404, "Not Found", QByteArray(), "text/plain");
    }
}

void SimpleHttpServer::sendResponse(QTcpSocket* socket, int statusCode, const QString& statusText,
                                   const QByteArray& content, const QString& contentType)
{
    QByteArray response;
    response += QString("HTTP/1.1 %1 %2\r\n").arg(statusCode).arg(statusText).toUtf8();
    response += QString("Content-Type: %1\r\n").arg(contentType).toUtf8();
    response += QString("Content-Length: %1\r\n").arg(content.size()).toUtf8();
    response += "Connection: close\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "\r\n";
    response += content;
    
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

void SimpleHttpServer::sendFile(QTcpSocket* socket, const QString& filePath, const QString& contentType)
{
    QString cleanPath = QDir::cleanPath(filePath);
    if (!cleanPath.startsWith(m_rootDir)) {
        sendResponse(socket, 403, "Forbidden", QByteArray(), "text/plain");
        return;
    }

    if (!isRealPathSafe(cleanPath)) {
        sendResponse(socket, 403, "Forbidden", QByteArray(), "text/plain");
        return;
    }

    QFile file(cleanPath);
    if (!file.open(QIODevice::ReadOnly)) {
        sendResponse(socket, 404, "Not Found", QByteArray(), "text/plain");
        return;
    }
    QByteArray content = file.readAll();
    file.close();
    sendResponse(socket, 200, "OK", content, contentType);
}

bool SimpleHttpServer::isPathSafe(const QString& path) const
{
    QString decoded = QUrl::fromPercentEncoding(path.toUtf8());

    if (decoded.contains("..")) return false;
    if (decoded.contains('\\')) return false;
    if (decoded.contains('\0')) return false;

    QString normalized = QDir::cleanPath(m_rootDir + "/" + decoded);
    if (!normalized.startsWith(m_rootDir)) return false;

    if (!path.isEmpty() && path[0] == '/') return false;

    return true;
}

bool SimpleHttpServer::isRealPathSafe(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) return false;

    QString canonical = fileInfo.canonicalFilePath();
    if (canonical.isEmpty()) return false;

    if (!canonical.startsWith(m_rootDir)) return false;

    return true;
}

// UploadToolPlugin 实现

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

    // 发布模式选择
    QGroupBox* modeGroup = new QGroupBox(tr("发布模式"));
    QHBoxLayout* modeLayout = new QHBoxLayout(modeGroup);
    modeLayout->setSpacing(10);
    
    modeLayout->addWidget(new QLabel(tr("模式:")));
    m_publishModeCombo = new QComboBox();
    m_publishModeCombo->addItem(tr("📁 本地服务器"), static_cast<int>(PublishMode::LocalServer));
    m_publishModeCombo->addItem(tr("🌐 远程服务器"), static_cast<int>(PublishMode::RemoteServer));
    m_publishModeCombo->addItem(tr("🖥️ 自身作为服务器"), static_cast<int>(PublishMode::SelfAsServer));
    connect(m_publishModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UploadToolPlugin::onPublishModeChanged);
    modeLayout->addWidget(m_publishModeCombo, 1);
    
    modeLayout->addWidget(new QLabel(tr("端口:")));
    m_selfServerPortSpin = new QSpinBox();
    m_selfServerPortSpin->setRange(1, 65535);
    m_selfServerPortSpin->setValue(8080);
    m_selfServerPortSpin->setEnabled(false);
    modeLayout->addWidget(m_selfServerPortSpin);
    
    mainLayout->addWidget(modeGroup);
    
    m_modeInfoLabel = new QLabel();
    m_modeInfoLabel->setStyleSheet("color: #666; font-style: italic;");
    mainLayout->addWidget(m_modeInfoLabel);

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
    m_publishModeCombo->setCurrentIndex(static_cast<int>(pc.publishMode));
    m_selfServerPortSpin->setValue(pc.selfServerPort);
    onPublishModeChanged(static_cast<int>(pc.publishMode));

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
    PublishMode mode = static_cast<PublishMode>(m_publishModeCombo->currentIndex());
    int port = m_selfServerPortSpin->value();

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
    if (serverDir.isEmpty() && mode != PublishMode::RemoteServer) {
        QMessageBox::warning(nullptr, tr("参数错误"), tr("请配置服务器目录"));
        return;
    }

    bool success = false;
    switch (mode) {
    case PublishMode::LocalServer:
        success = publishToLocalServer(appName, version, exePath, serverDir, serverUrl, releaseNotes);
        break;
    case PublishMode::RemoteServer:
        success = publishToRemoteServer(appName, version, exePath, serverDir, serverUrl, releaseNotes);
        break;
    case PublishMode::SelfAsServer:
        success = publishAsSelfServer(appName, version, exePath, serverDir, port, releaseNotes);
        break;
    }

    if (success) {
        m_outputEdit->append(tr("\n✅ 发布完成！"));
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
    const qint64 chunkSize = 65536;
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

void UploadToolPlugin::onPublishModeChanged(int index)
{
    PublishMode mode = static_cast<PublishMode>(index);
    
    switch (mode) {
    case PublishMode::LocalServer:
        m_modeInfoLabel->setText(tr("📁 本地服务器模式：将 EXE 文件复制到本地服务器目录，适合局域网内共享"));
        m_selfServerPortSpin->setEnabled(false);
        break;
    case PublishMode::RemoteServer:
        m_modeInfoLabel->setText(tr("🌐 远程服务器模式：通过 HTTP POST 上传到远程更新服务器"));
        m_selfServerPortSpin->setEnabled(false);
        break;
    case PublishMode::SelfAsServer:
        m_modeInfoLabel->setText(tr("🖥️ 自身作为服务器模式：启动内置 HTTP 服务器，直接提供更新服务"));
        m_selfServerPortSpin->setEnabled(true);
        // 自动更新 URL
        m_serverUrlEdit->setText(QString("http://localhost:%1").arg(m_selfServerPortSpin->value()));
        break;
    }
}

bool UploadToolPlugin::publishToLocalServer(const QString& appName, const QString& version, 
                                           const QString& exePath, const QString& serverDir,
                                           const QString& serverUrl, const QString& releaseNotes)
{
    m_outputEdit->append(tr("=== 本地服务器模式 ==="));
    
    m_outputEdit->append(tr("\n[1/4] 计算 SHA-256 哈希..."));
    QString hash = calculateSHA256(exePath);
    if (hash.isEmpty()) {
        m_outputEdit->append(tr("  ✗ 计算哈希失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法计算 EXE 文件的 SHA-256 哈希"));
        return false;
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
        return false;
    }
    m_outputEdit->append(tr("  ✓ 已创建目录: %1").arg(packagesDir));

    if (QFile::exists(destExePath)) {
        int ret = QMessageBox::question(nullptr, tr("文件已存在"),
            tr("服务器已存在同名文件:\n%1\n\n是否覆盖？").arg(destExePath),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            m_outputEdit->append(tr("  ✗ 用户取消覆盖"));
            return false;
        }
        if (!QFile::remove(destExePath)) {
            m_outputEdit->append(tr("  ✗ 删除旧文件失败"));
            QMessageBox::critical(nullptr, tr("错误"), tr("无法删除服务器上的旧文件"));
            return false;
        }
        m_outputEdit->append(tr("  ✓ 已删除旧文件"));
    }

    if (!copyExeToServer(exePath, destExePath)) {
        m_outputEdit->append(tr("  ✗ 复制文件失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法复制 EXE 文件到服务器"));
        return false;
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
        return false;
    }
    m_outputEdit->append(tr("  ✓ version.json 已更新"));

    m_outputEdit->append(tr("\n[4/4] 验证发布结果..."));
    if (QFile::exists(destExePath) && QFile::exists(versionJsonPath)) {
        m_outputEdit->append(tr("  ✓ 验证通过！文件已正确上传"));
        m_outputEdit->append(tr("  下载 URL: %1/%2/packages/%3").arg(serverUrl, appName, exeFileName));
        QMessageBox::information(nullptr, tr("发布成功"),
            tr("版本 %1 已成功发布到本地服务器\n\n下载 URL:\n%2").arg(version, serverUrl + "/" + appName + "/packages/" + exeFileName));
        return true;
    } else {
        m_outputEdit->append(tr("  ✗ 验证失败：文件不存在"));
        QMessageBox::warning(nullptr, tr("发布警告"), tr("发布可能未成功完成，请检查服务器目录"));
        return false;
    }
}

bool UploadToolPlugin::publishToRemoteServer(const QString& appName, const QString& version,
                                            const QString& exePath, const QString& serverDir,
                                            const QString& serverUrl, const QString& releaseNotes)
{
    m_outputEdit->append(tr("=== 远程服务器模式 ==="));

    m_outputEdit->append(tr("\n[1/5] 计算 SHA-256 哈希..."));
    QString hash = calculateSHA256(exePath);
    if (hash.isEmpty()) {
        m_outputEdit->append(tr("  ✗ 计算哈希失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法计算 EXE 文件的 SHA-256 哈希"));
        return false;
    }
    m_outputEdit->append(tr("  ✓ SHA256: %1").arg(hash));

    QFileInfo exeFileInfo(exePath);
    QString exeFileName = exeFileInfo.fileName();

    // 先保存到本地缓存目录
    QString localCacheDir = serverDir;
    if (!localCacheDir.isEmpty()) {
        QString packagesDir = QDir(localCacheDir).filePath(appName + "/packages");
        QString destExePath = QDir(packagesDir).filePath(exeFileName);

        m_outputEdit->append(tr("\n[2/5] 保存到本地缓存..."));
        if (QDir().mkpath(packagesDir)) {
            copyExeToServer(exePath, destExePath);
            m_outputEdit->append(tr("  ✓ 已缓存到: %1").arg(destExePath));
        }
    } else {
        m_outputEdit->append(tr("\n[2/5] 跳过本地缓存（未配置缓存目录）"));
    }

    m_outputEdit->append(tr("\n[3/5] 上传到远程服务器..."));
    m_outputEdit->append(tr("  服务器: %1").arg(serverUrl));

    // 创建网络管理器（如果不存在）
    if (!m_networkManager) {
        m_networkManager = new QNetworkAccessManager(this);
    }

    // 保存上传信息用于回调
    m_pendingAppName = appName;
    m_pendingVersion = version;
    m_pendingExeFileName = exeFileName;
    m_pendingServerUrl = serverUrl;

    QUrl uploadUrl(QString("%1/%2/upload").arg(serverUrl, appName));
    QNetworkRequest request(uploadUrl);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 添加文件
    QFile* file = new QFile(exePath);
    if (!file->open(QIODevice::ReadOnly)) {
        m_outputEdit->append(tr("  ✗ 无法打开文件"));
        delete file;
        delete multiPart;
        return false;
    }

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(exeFileName)));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    // 添加元数据
    QHttpPart versionPart;
    versionPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"version\""));
    versionPart.setBody(version.toUtf8());
    multiPart->append(versionPart);

    QHttpPart hashPart;
    hashPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"hash\""));
    hashPart.setBody(QString("sha256:" + hash).toUtf8());
    multiPart->append(hashPart);

    QHttpPart notesPart;
    notesPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"notes\""));
    notesPart.setBody(releaseNotes.toUtf8());
    multiPart->append(notesPart);

    // 异步上传 - 不阻塞 UI
    m_currentUploadReply = m_networkManager->post(request, multiPart);
    multiPart->setParent(m_currentUploadReply);

    // 连接进度和完成信号
    connect(m_currentUploadReply, &QNetworkReply::uploadProgress,
            this, &UploadToolPlugin::onUploadProgress);
    connect(m_currentUploadReply, &QNetworkReply::finished,
            this, &UploadToolPlugin::onUploadFinished);

    // 禁用发布按钮防止重复操作
    m_publishBtn->setEnabled(false);
    m_outputEdit->append(tr("  ⏳ 上传进行中，请等待..."));

    // 返回 true 表示上传已启动（实际结果在回调中处理）
    return true;
}

void UploadToolPlugin::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int percent = static_cast<int>((bytesSent * 100) / bytesTotal);
        // 只在进度变化超过 10% 时记录日志，避免日志过多
        if (percent - m_lastReportedPercent >= 10 || percent == 100) {
            m_outputEdit->append(tr("  进度: %1% (%2 / %3 KB)")
                .arg(percent)
                .arg(bytesSent / 1024)
                .arg(bytesTotal / 1024));
            m_lastReportedPercent = percent;
        }
    }
}

void UploadToolPlugin::onUploadFinished()
{
    if (!m_currentUploadReply) return;

    m_publishBtn->setEnabled(true);

    if (m_currentUploadReply->error() != QNetworkReply::NoError) {
        QString errorMsg = tr("上传失败: %1").arg(m_currentUploadReply->errorString());
        m_outputEdit->append(tr("  ✗ %1").arg(errorMsg));
        QMessageBox::critical(nullptr, tr("上传失败"),
            tr("无法上传到远程服务器: %1").arg(m_currentUploadReply->errorString()));
        m_currentUploadReply->deleteLater();
        m_currentUploadReply = nullptr;
        emit uploadFinished(false, errorMsg);
        return;
    }

    m_outputEdit->append(tr("  ✓ 上传成功"));
    m_outputEdit->append(tr("\n[4/5] 验证远程服务器..."));

    QUrl versionUrl(QString("%1/%2/version.json").arg(m_pendingServerUrl, m_pendingAppName));
    QNetworkRequest versionRequest(versionUrl);
    QNetworkReply* versionReply = m_networkManager->get(versionRequest);

    connect(versionReply, &QNetworkReply::finished, this, [this, versionReply]() {
        if (versionReply->error() == QNetworkReply::NoError) {
            m_outputEdit->append(tr("  ✓ version.json 可访问"));
        } else {
            m_outputEdit->append(tr("  ⚠ 无法验证 version.json"));
        }
        versionReply->deleteLater();

        m_outputEdit->append(tr("\n[5/5] 发布完成"));
        m_outputEdit->append(tr("  下载 URL: %1/%2/packages/%3")
            .arg(m_pendingServerUrl, m_pendingAppName, m_pendingExeFileName));
        m_outputEdit->append(tr("\n✅ 发布完成！"));

        QString successMsg = tr("版本 %1 已成功上传到远程服务器\n\n下载 URL:\n%2")
            .arg(m_pendingVersion, m_pendingServerUrl + "/" + m_pendingAppName + "/packages/" + m_pendingExeFileName);
        QMessageBox::information(nullptr, tr("发布成功"), successMsg);
        emit uploadFinished(true, successMsg);
    });

    m_currentUploadReply->deleteLater();
    m_currentUploadReply = nullptr;
}

bool UploadToolPlugin::publishAsSelfServer(const QString& appName, const QString& version,
                                          const QString& exePath, const QString& serverDir,
                                          int port, const QString& releaseNotes)
{
    m_outputEdit->append(tr("=== 自身作为服务器模式 ==="));
    
    m_outputEdit->append(tr("\n[1/5] 计算 SHA-256 哈希..."));
    QString hash = calculateSHA256(exePath);
    if (hash.isEmpty()) {
        m_outputEdit->append(tr("  ✗ 计算哈希失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法计算 EXE 文件的 SHA-256 哈希"));
        return false;
    }
    m_outputEdit->append(tr("  ✓ SHA256: %1").arg(hash));

    QString packagesDir = QDir(serverDir).filePath(appName + "/packages");
    QFileInfo exeFileInfo(exePath);
    QString exeFileName = exeFileInfo.fileName();
    QString destExePath = QDir(packagesDir).filePath(exeFileName);

    m_outputEdit->append(tr("\n[2/5] 复制 EXE 到服务目录..."));
    m_outputEdit->append(tr("  目标: %1").arg(destExePath));

    if (!QDir().mkpath(packagesDir)) {
        m_outputEdit->append(tr("  ✗ 创建目录失败: %1").arg(packagesDir));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法创建服务目录: %1").arg(packagesDir));
        return false;
    }
    m_outputEdit->append(tr("  ✓ 已创建目录: %1").arg(packagesDir));

    if (!copyExeToServer(exePath, destExePath)) {
        m_outputEdit->append(tr("  ✗ 复制文件失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法复制 EXE 文件"));
        return false;
    }
    m_outputEdit->append(tr("  ✓ 已复制到: %1").arg(destExePath));

    qint64 fileSize = exeFileInfo.size();
    QString versionJsonPath = QDir(serverDir).filePath(appName + "/version.json");
    QString serverUrl = QString("http://localhost:%1").arg(port);

    m_outputEdit->append(tr("\n[3/5] 更新 version.json..."));
    m_outputEdit->append(tr("  文件: %1").arg(versionJsonPath));

    if (!updateVersionJson(versionJsonPath, appName, version, exeFileName, fileSize,
                          "sha256:" + hash, serverUrl, releaseNotes)) {
        m_outputEdit->append(tr("  ✗ 更新 version.json 失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法更新 version.json 文件"));
        return false;
    }
    m_outputEdit->append(tr("  ✓ version.json 已更新"));

    m_outputEdit->append(tr("\n[4/5] 启动内置 HTTP 服务器..."));
    startSelfServer(serverDir, port);
    
    if (m_httpServer) {
        m_outputEdit->append(tr("  ✓ HTTP 服务器已启动，监听端口: %1").arg(port));
    } else {
        m_outputEdit->append(tr("  ✗ 启动 HTTP 服务器失败"));
        QMessageBox::critical(nullptr, tr("错误"), tr("无法启动内置 HTTP 服务器"));
        return false;
    }

    m_outputEdit->append(tr("\n[5/5] 发布完成"));
    m_outputEdit->append(tr("  访问 URL: %1").arg(serverUrl));
    m_outputEdit->append(tr("  version.json: %1/%2/version.json").arg(serverUrl, appName));
    m_outputEdit->append(tr("  下载 URL: %1/%2/packages/%3").arg(serverUrl, appName, exeFileName));
    
    QMessageBox::information(nullptr, tr("发布成功"),
        tr("版本 %1 已成功发布\n\n内置 HTTP 服务器已启动\n监听端口: %2\n\n访问 URL:\n%3").arg(version).arg(port).arg(serverUrl));
    return true;
}

void UploadToolPlugin::startSelfServer(const QString& rootDir, int port)
{
    // 停止现有服务器
    stopSelfServer();
    
    // 创建 HTTP 服务器
    m_httpServer = new SimpleHttpServer(this);
    
    if (!m_httpServer->start(rootDir, port)) {
        m_outputEdit->append(tr("  ✗ 无法监听端口 %1").arg(port));
        delete m_httpServer;
        m_httpServer = nullptr;
        return;
    }
}

void UploadToolPlugin::stopSelfServer()
{
    if (m_httpServer) {
        m_httpServer->stop();
        delete m_httpServer;
        m_httpServer = nullptr;
    }
}
