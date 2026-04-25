#pragma once

#include "upload_config.h"
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QSpinBox>

class ProjectManageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectManageDialog(QWidget* parent = nullptr);

signals:
    void configUpdated();

private slots:
    void addProject();
    void editProject();
    void deleteProject();
    void selectServerDir();
    void selectDefaultExePath();
    void onProjectSelectionChanged();
    void saveProject();
    void onPublishModeChanged(int index);

private:
    void loadProjectsList();
    void populateForm(const ProjectConfig& config);
    void clearForm();
    ProjectConfig getFormConfig() const;
    bool validateConfig();

    QListWidget* m_projectsList = nullptr;
    QLineEdit* m_appNameEdit = nullptr;
    QLineEdit* m_serverDirEdit = nullptr;
    QLineEdit* m_serverUrlEdit = nullptr;
    QLineEdit* m_defaultExePathEdit = nullptr;
    QLineEdit* m_versionPrefixEdit = nullptr;
    QComboBox* m_publishModeCombo = nullptr;
    QSpinBox* m_selfServerPortSpin = nullptr;
    QLabel* m_serverDirLabel = nullptr;
    QLabel* m_serverUrlLabel = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;
    
    int m_currentIndex = -1;
};
