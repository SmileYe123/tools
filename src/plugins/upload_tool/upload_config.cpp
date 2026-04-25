#include "upload_config.h"

UploadConfig* UploadConfig::instance()
{
    static UploadConfig inst;
    return &inst;
}

QString UploadConfig::configFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(configDir);
    return configDir + "/upload_config.json";
}

bool UploadConfig::load()
{
    QString path = configFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    m_lastUsedProject = root.value("last_used_project").toString();
    
    QJsonArray projectsArray = root.value("projects").toArray();
    m_projects.clear();
    for (const QJsonValue& val : projectsArray) {
        if (val.isObject()) {
            m_projects.append(jsonToProject(val.toObject()));
        }
    }
    
    return true;
}

bool UploadConfig::save()
{
    QJsonObject root;
    root["last_used_project"] = m_lastUsedProject;
    
    QJsonArray projectsArray;
    for (const ProjectConfig& config : m_projects) {
        projectsArray.append(projectToJson(config));
    }
    root["projects"] = projectsArray;
    
    QJsonDocument doc(root);
    QString path = configFilePath();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

ProjectConfig UploadConfig::projectAt(int index) const
{
    if (index >= 0 && index < m_projects.size()) {
        return m_projects[index];
    }
    return ProjectConfig();
}

int UploadConfig::addProject(const ProjectConfig& config)
{
    m_projects.append(config);
    return m_projects.size() - 1;
}

bool UploadConfig::updateProject(int index, const ProjectConfig& config)
{
    if (index >= 0 && index < m_projects.size()) {
        m_projects[index] = config;
        return true;
    }
    return false;
}

bool UploadConfig::removeProject(int index)
{
    if (index >= 0 && index < m_projects.size()) {
        m_projects.removeAt(index);
        return true;
    }
    return false;
}

int UploadConfig::findProject(const QString& appName) const
{
    for (int i = 0; i < m_projects.size(); ++i) {
        if (m_projects[i].appName == appName) {
            return i;
        }
    }
    return -1;
}

void UploadConfig::setLastUsedProject(const QString& appName)
{
    m_lastUsedProject = appName;
}

QJsonObject UploadConfig::projectToJson(const ProjectConfig& config) const
{
    QJsonObject obj;
    obj["app_name"] = config.appName;
    obj["server_dir"] = config.serverDir;
    obj["server_url"] = config.serverUrl;
    obj["default_exe_path"] = config.defaultExePath;
    obj["version_prefix"] = config.versionPrefix;
    obj["publish_mode"] = static_cast<int>(config.publishMode);
    obj["self_server_port"] = config.selfServerPort;
    return obj;
}

ProjectConfig UploadConfig::jsonToProject(const QJsonObject& obj) const
{
    ProjectConfig config;
    config.appName = obj.value("app_name").toString();
    config.serverDir = obj.value("server_dir").toString();
    config.serverUrl = obj.value("server_url").toString();
    config.defaultExePath = obj.value("default_exe_path").toString();
    config.versionPrefix = obj.value("version_prefix").toString("V");
    config.publishMode = static_cast<PublishMode>(obj.value("publish_mode").toInt(0));
    config.selfServerPort = obj.value("self_server_port").toInt(8080);
    return config;
}
