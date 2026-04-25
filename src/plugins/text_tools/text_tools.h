#pragma once

#include "plugins/base.h"
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <algorithm>
#include <QRegularExpression>

class TextToolPlugin : public BasePlugin
{
    Q_OBJECT

public:
    explicit TextToolPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        setName(tr("文本工具"));
        setIcon("📝");
        setDescription(tr("文本格式转换、大小写转换、统计等"));
        setCategory("text");
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* inputGroup = new QGroupBox(tr("输入文本"));
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
        m_inputEdit = new QTextEdit();
        m_inputEdit->setPlaceholderText(tr("在此输入或粘贴文本..."));
        inputLayout->addWidget(m_inputEdit);
        layout->addWidget(inputGroup);

        QHBoxLayout* operationsLayout = new QHBoxLayout();
        operationsLayout->setSpacing(8);

        m_formatCombo = new QComboBox();
        m_formatCombo->addItems({tr("转大写"), tr("转小写"), tr("去除首尾空格"), tr("去除所有空格"), tr("去除空行"), tr("反转文本"), tr("统计字符"), tr("排序行")});
        operationsLayout->addWidget(m_formatCombo);

        QPushButton* executeBtn = new QPushButton(tr("▶ 执行"));
        connect(executeBtn, &QPushButton::clicked, this, &TextToolPlugin::executeOperation);
        operationsLayout->addWidget(executeBtn);

        QPushButton* clearBtn = new QPushButton(tr("🗑️ 清空"));
        connect(clearBtn, &QPushButton::clicked, this, &TextToolPlugin::clear);
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
    void executeOperation()
    {
        QString text = m_inputEdit->toPlainText();
        QString operation = m_formatCombo->currentText();
        QString result;

        if (operation == tr("转大写")) {
            result = text.toUpper();
        } else if (operation == tr("转小写")) {
            result = text.toLower();
        } else if (operation == tr("去除首尾空格")) {
            QStringList lines = text.split("\n");
            QStringList trimmed;
            for (const QString& line : lines) trimmed.append(line.trimmed());
            result = trimmed.join("\n");
        } else if (operation == tr("去除所有空格")) {
            result = text.remove(" ").remove("\t");
        } else if (operation == tr("去除空行")) {
            QStringList lines = text.split("\n");
            QStringList nonEmpty;
            for (const QString& line : lines) {
                if (!line.trimmed().isEmpty()) nonEmpty.append(line);
            }
            result = nonEmpty.join("\n");
        } else if (operation == tr("反转文本")) {
            result = text;
            std::reverse(result.begin(), result.end());
        } else if (operation == tr("统计字符")) {
            int chars = text.length();
            int lines = text.split("\n").size();
            int words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
            result = tr("字符数: %1\n行数: %2\n单词数: %3").arg(chars).arg(lines).arg(words);
        } else if (operation == tr("排序行")) {
            QStringList lines = text.split("\n");
            std::sort(lines.begin(), lines.end());
            result = lines.join("\n");
        }

        m_outputEdit->setText(result);
    }

    void clear() { m_inputEdit->clear(); m_outputEdit->clear(); }

private:
    QTextEdit* m_inputEdit = nullptr;
    QTextEdit* m_outputEdit = nullptr;
    QComboBox* m_formatCombo = nullptr;
};
