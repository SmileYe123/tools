#pragma once

#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include "plugins/base.h"

class Sidebar : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);

    void setupToolButtons(const QList<BasePlugin*>& plugins);
    void selectTool(int index);

signals:
    void toolSelected(int index);

private slots:
    void filterTools();

private:
    QVBoxLayout* m_mainLayout = nullptr;
    QVBoxLayout* m_buttonsLayout = nullptr;
    QScrollArea* m_scrollArea = nullptr;
    QWidget* m_scrollWidget = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QList<QToolButton*> m_toolButtons;
};
