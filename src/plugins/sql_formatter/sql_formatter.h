#pragma once

#include "plugins/base.h"
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRegularExpression>

class SqlFormatterPlugin : public BasePlugin
{
    Q_OBJECT

public:
    SqlFormatterPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        m_name = tr("SQL格式化");
        m_icon = "🗄️";
        m_description = tr("SQL语句格式化和美化");
        m_category = "data";
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* inputGroup = new QGroupBox(tr("SQL输入"));
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
        m_inputEdit = new QTextEdit();
        m_inputEdit->setPlaceholderText(tr("在此粘贴SQL语句..."));
        inputLayout->addWidget(m_inputEdit);
        layout->addWidget(inputGroup);

        QHBoxLayout* operationsLayout = new QHBoxLayout();
        operationsLayout->setSpacing(8);
        operationsLayout->addWidget(new QLabel(tr("缩进:")));
        m_indentCombo = new QComboBox();
        m_indentCombo->addItems({"2 " + tr("空格"), "4 " + tr("空格"), "Tab"});
        operationsLayout->addWidget(m_indentCombo);
        QPushButton* formatBtn = new QPushButton(tr("✨ 格式化"));
        connect(formatBtn, &QPushButton::clicked, this, &SqlFormatterPlugin::formatSql);
        operationsLayout->addWidget(formatBtn);
        QPushButton* compressBtn = new QPushButton(tr("🗜️ 压缩"));
        connect(compressBtn, &QPushButton::clicked, this, &SqlFormatterPlugin::compressSql);
        operationsLayout->addWidget(compressBtn);
        QPushButton* clearBtn = new QPushButton(tr("🗑️ 清空"));
        connect(clearBtn, &QPushButton::clicked, this, &SqlFormatterPlugin::clear);
        operationsLayout->addWidget(clearBtn);
        layout->addLayout(operationsLayout);

        QGroupBox* outputGroup = new QGroupBox(tr("格式化结果"));
        QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
        m_outputEdit = new QTextEdit();
        m_outputEdit->setReadOnly(true);
        m_outputEdit->setPlaceholderText(tr("格式化结果将显示在这里..."));
        outputLayout->addWidget(m_outputEdit);
        layout->addWidget(outputGroup);

        return widget;
    }

private slots:
    void formatSql()
    {
        QString sql = m_inputEdit->toPlainText().trimmed();
        if (sql.isEmpty()) { m_outputEdit->setText(tr("请输入SQL语句")); return; }

        QString indent = "  ";
        if (m_indentCombo->currentIndex() == 1) indent = "    ";
        else if (m_indentCombo->currentIndex() == 2) indent = "\t";

        QString formatted = sql.replace(QRegularExpression("\\s+"), " ").trimmed();
        QStringList keywords = {"SELECT", "FROM", "WHERE", "JOIN", "LEFT JOIN", "RIGHT JOIN", "INNER JOIN", "ON", "AND", "OR", "GROUP BY", "ORDER BY", "HAVING", "LIMIT", "INSERT INTO", "VALUES", "UPDATE", "SET", "DELETE FROM"};
        for (const QString& keyword : keywords) {
            QRegularExpression regex(QString("\\b%1\\b").arg(keyword), QRegularExpression::CaseInsensitiveOption);
            formatted.replace(regex, "\n" + keyword.toUpper());
        }
        m_outputEdit->setText(formatted);
    }

    void compressSql()
    {
        QString sql = m_inputEdit->toPlainText().trimmed();
        if (sql.isEmpty()) { m_outputEdit->setText(tr("请输入SQL语句")); return; }
        m_outputEdit->setText(sql.replace(QRegularExpression("\\s+"), " ").trimmed());
    }

    void clear() { m_inputEdit->clear(); m_outputEdit->clear(); }

private:
    QTextEdit* m_inputEdit = nullptr;
    QTextEdit* m_outputEdit = nullptr;
    QComboBox* m_indentCombo = nullptr;
};
