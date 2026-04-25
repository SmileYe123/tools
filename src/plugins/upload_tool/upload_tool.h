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

class UploadToolPlugin : public BasePlugin
{
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

private:
    QString calculateSHA256(const QString& filePath);
    bool copyExeToServer(const QString& srcPath, const QString& destPath);
    bool updateVersionJson(const QString& jsonPath, const QString& appName,
                          const QString& version, const QString& filename,
                          qint64 fileSize, const QString& hash, const QString& serverUrl,
                          const QString& releaseNotes);
    void autoExtractVersion(const QString& exePath);
    
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
    QWidget* m_widget = nullptr;
    
    bool m_isLoadingConfig = false;
};
