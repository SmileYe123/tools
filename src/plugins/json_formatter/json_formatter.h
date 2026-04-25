#pragma once

#include "plugins/base.h"
#include <QTextEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

class JsonFormatterPlugin : public BasePlugin
{
public:
    explicit JsonFormatterPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        m_name = tr("JSON格式化");
        m_icon = "📋";
        m_description = tr("JSON格式化、压缩、验证工具");
        m_category = "text";
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* inputGroup = new QGroupBox(tr("JSON输入"));
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
        m_inputEdit = new QTextEdit();
        m_inputEdit->setPlaceholderText(tr("在此粘贴JSON文本..."));
        inputLayout->addWidget(m_inputEdit);
        layout->addWidget(inputGroup);

        QHBoxLayout* operationsLayout = new QHBoxLayout();
        operationsLayout->setSpacing(8);

        QLabel* indentLabel = new QLabel(tr("缩进:"));
        operationsLayout->addWidget(indentLabel);

        m_indentSpin = new QSpinBox();
        m_indentSpin->setRange(1, 8);
        m_indentSpin->setValue(2);
        operationsLayout->addWidget(m_indentSpin);

        QPushButton* formatBtn = new QPushButton(tr("✨ 格式化"));
        connect(formatBtn, &QPushButton::clicked, this, &JsonFormatterPlugin::formatJson);
        operationsLayout->addWidget(formatBtn);

        QPushButton* compressBtn = new QPushButton(tr("🗜️ 压缩"));
        connect(compressBtn, &QPushButton::clicked, this, &JsonFormatterPlugin::compressJson);
        operationsLayout->addWidget(compressBtn);

        QPushButton* validateBtn = new QPushButton(tr("✓ 验证"));
        connect(validateBtn, &QPushButton::clicked, this, &JsonFormatterPlugin::validateJson);
        operationsLayout->addWidget(validateBtn);

        QPushButton* clearBtn = new QPushButton(tr("🗑️ 清空"));
        connect(clearBtn, &QPushButton::clicked, this, &JsonFormatterPlugin::clear);
        operationsLayout->addWidget(clearBtn);

        layout->addLayout(operationsLayout);

        QGroupBox* outputGroup = new QGroupBox(tr("输出结果"));
        QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
        m_outputEdit = new QTextEdit();
        m_outputEdit->setReadOnly(true);
        m_outputEdit->setPlaceholderText(tr("结果将显示在这里..."));
        outputLayout->addWidget(m_outputEdit);
        layout->addWidget(outputGroup);

        return widget;
    }

private slots:
    void formatJson()
    {
        QString text = m_inputEdit->toPlainText().trimmed();
        if (text.isEmpty()) { m_outputEdit->setText(tr("请输入JSON文本")); return; }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            m_outputEdit->setText(tr("JSON解析错误:\n") + error.errorString());
            return;
        }

        int indent = m_indentSpin->value();
        QByteArray formatted = doc.toJson(QJsonDocument::Indented);
        QString formattedStr = QString::fromUtf8(formatted);
        if (indent != 4) { formattedStr.replace("    ", QString(indent, ' ')); }
        m_outputEdit->setText(formattedStr);
    }

    void compressJson()
    {
        QString text = m_inputEdit->toPlainText().trimmed();
        if (text.isEmpty()) { m_outputEdit->setText(tr("请输入JSON文本")); return; }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            m_outputEdit->setText(tr("JSON解析错误:\n") + error.errorString());
            return;
        }

        m_outputEdit->setText(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    }

    void validateJson()
    {
        QString text = m_inputEdit->toPlainText().trimmed();
        if (text.isEmpty()) { m_outputEdit->setText(tr("请输入JSON文本")); return; }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            m_outputEdit->setText(tr("✗ 无效的JSON\n错误: ") + error.errorString());
            return;
        }

        if (doc.isObject()) {
            m_outputEdit->setText(tr("✓ 有效的JSON对象"));
        } else if (doc.isArray()) {
            m_outputEdit->setText(tr("✓ 有效的JSON数组"));
        }
    }

    void clear() { m_inputEdit->clear(); m_outputEdit->clear(); }

private:
    QTextEdit* m_inputEdit = nullptr;
    QTextEdit* m_outputEdit = nullptr;
    QSpinBox* m_indentSpin = nullptr;
};
