#pragma once

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

// 发布模式枚举
enum class PublishMode {
    LocalServer = 0,    // 本地服务器模式：复制到本地目录
    RemoteServer = 1,   // 远程服务器模式：通过 HTTP 上传到远程服务器
    SelfAsServer = 2    // 自身作为服务器模式：启动内置 HTTP 服务器
};

struct ProjectConfig {
    QString appName;
    QString serverDir;
    QString serverUrl;
    QString defaultExePath;
    QString versionPrefix;
    PublishMode publishMode = PublishMode::LocalServer;  // 发布模式
    int selfServerPort = 8080;  // 自身作为服务器时的端口
};

class UploadConfig
{
public:
    static UploadConfig* instance();

    UploadConfig(const UploadConfig&) = delete;
    UploadConfig& operator=(const UploadConfig&) = delete;

    bool load();
    bool save();

    QList<ProjectConfig> projects() const { return m_projects; }
    ProjectConfig projectAt(int index) const;
    int projectCount() const { return m_projects.size(); }

    int addProject(const ProjectConfig& config);
    bool updateProject(int index, const ProjectConfig& config);
    bool removeProject(int index);
    int findProject(const QString& appName) const;

    QString lastUsedProject() const { return m_lastUsedProject; }
    void setLastUsedProject(const QString& appName);

    QString configFilePath() const;

private:
    UploadConfig() = default;

    QJsonObject projectToJson(const ProjectConfig& config) const;
    ProjectConfig jsonToProject(const QJsonObject& obj) const;

    QList<ProjectConfig> m_projects;
    QString m_lastUsedProject;
};
