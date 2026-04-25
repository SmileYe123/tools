#pragma once

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

struct ProjectConfig {
    QString appName;
    QString serverDir;
    QString serverUrl;
    QString defaultExePath;
    QString versionPrefix;
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
