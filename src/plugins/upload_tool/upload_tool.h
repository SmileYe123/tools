#pragma once

#include "plugins/base.h"
#include "upload_config.h"
#include "project_manage_dialog.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QCryptographicHash>
#include <QProgressDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QUrl>

// 简单的 HTTP 服务器类
class SimpleHttpServer : public QObject
{
    Q_OBJECT
public:
    explicit SimpleHttpServer(QObject* parent = nullptr);
    ~SimpleHttpServer();
    bool start(const QString& rootDir, quint16 port);
    void stop();
    bool isRunning() const { return m_server && m_server->isListening(); }
    quint16 serverPort() const { return m_server ? m_server->serverPort() : 0; }

signals:
    void error(const QString& message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    void handleRequest(QTcpSocket* socket, const QString& method, const QString& path);
    void sendResponse(QTcpSocket* socket, int statusCode, const QString& statusText,
                     const QByteArray& content, const QString& contentType);
    void sendFile(QTcpSocket* socket, const QString& filePath, const QString& contentType);
    bool isPathSafe(const QString& path) const;

    QTcpServer* m_server = nullptr;
    QString m_rootDir;
    QMap<QTcpSocket*, QByteArray> m_buffers;
};

class UploadToolPlugin : public BasePlugin
{
    Q_OBJECT

public:
    explicit UploadToolPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        setName(tr("上传到更新服务器"));
        setIcon("🚀");
        setDescription(tr("发布新版本到自动更新服务器"));
        setCategory("system");
    }

    QWidget* createWidget(QWidget* parent = nullptr) override;

private slots:
    void onProjectSelected(int index);
    void selectExeFile();
    void openProjectManager();
    void publishUpdate();
    void clearForm();
    void onPublishModeChanged(int index);

private:
    QString calculateSHA256(const QString& filePath);
    bool copyExeToServer(const QString& srcPath, const QString& destPath);
    bool updateVersionJson(const QString& jsonPath, const QString& appName,
                          const QString& version, const QString& filename,
                          qint64 fileSize, const QString& hash, const QString& serverUrl,
                          const QString& releaseNotes);
    void autoExtractVersion(const QString& exePath);
    
    // 三种发布模式的实现
    bool publishToLocalServer(const QString& appName, const QString& version, 
                             const QString& exePath, const QString& serverDir,
                             const QString& serverUrl, const QString& releaseNotes);
    bool publishToRemoteServer(const QString& appName, const QString& version,
                              const QString& exePath, const QString& serverDir,
                              const QString& serverUrl, const QString& releaseNotes);
    bool publishAsSelfServer(const QString& appName, const QString& version,
                            const QString& exePath, const QString& serverDir,
                            int port, const QString& releaseNotes);
    
    // HTTP 服务器相关
    void startSelfServer(const QString& rootDir, int port);
    void stopSelfServer();
    
    QComboBox* m_projectCombo = nullptr;
    QPushButton* m_manageProjectsBtn = nullptr;
    QLineEdit* m_appNameEdit = nullptr;
    QLineEdit* m_versionEdit = nullptr;
    QLineEdit* m_exePathEdit = nullptr;
    QLineEdit* m_serverDirEdit = nullptr;
    QLineEdit* m_serverUrlEdit = nullptr;
    QTextEdit* m_releaseNotesEdit = nullptr;
    QTextEdit* m_outputEdit = nullptr;
    QPushButton* m_publishBtn = nullptr;
    QPushButton* m_clearBtn = nullptr;
    QComboBox* m_publishModeCombo = nullptr;
    QSpinBox* m_selfServerPortSpin = nullptr;
    QLabel* m_modeInfoLabel = nullptr;
    QWidget* m_widget = nullptr;
    
    bool m_isLoadingConfig = false;
    
    // HTTP 服务器
    SimpleHttpServer* m_httpServer = nullptr;
};
