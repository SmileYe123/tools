#pragma once

#include "plugins/base.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextCursor>

class RegexToolPlugin : public BasePlugin
{
    Q_OBJECT

public:
    RegexToolPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        setName(tr("正则工具"));
        setIcon("🔍");
        setDescription(tr("正则表达式测试、匹配和替换"));
        setCategory("text");
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* regexGroup = new QGroupBox(tr("正则表达式"));
        QVBoxLayout* regexLayout = new QVBoxLayout(regexGroup);
        QHBoxLayout* regexInputLayout = new QHBoxLayout();
        m_regexEdit = new QLineEdit();
        m_regexEdit->setPlaceholderText(tr("输入正则表达式..."));
        regexInputLayout->addWidget(m_regexEdit);
        m_caseSensitiveCb = new QCheckBox(tr("区分大小写"));
        regexInputLayout->addWidget(m_caseSensitiveCb);
        regexLayout->addLayout(regexInputLayout);
        layout->addWidget(regexGroup);

        QGroupBox* inputGroup = new QGroupBox(tr("测试文本"));
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
        m_inputEdit = new QTextEdit();
        m_inputEdit->setPlaceholderText(tr("在此输入测试文本..."));
        inputLayout->addWidget(m_inputEdit);
        layout->addWidget(inputGroup);

        QHBoxLayout* operationsLayout = new QHBoxLayout();
        operationsLayout->setSpacing(8);
        QPushButton* matchBtn = new QPushButton(tr("🎯 匹配"));
        connect(matchBtn, &QPushButton::clicked, this, &RegexToolPlugin::findMatches);
        operationsLayout->addWidget(matchBtn);
        operationsLayout->addWidget(new QLabel(tr("替换为:")));
        m_replaceEdit = new QLineEdit();
        m_replaceEdit->setPlaceholderText(tr("替换文本..."));
        operationsLayout->addWidget(m_replaceEdit);
        QPushButton* replaceBtn = new QPushButton(tr("🔄 替换"));
        connect(replaceBtn, &QPushButton::clicked, this, &RegexToolPlugin::replaceMatches);
        operationsLayout->addWidget(replaceBtn);
        QPushButton* clearBtn = new QPushButton(tr("🗑️ 清空"));
        connect(clearBtn, &QPushButton::clicked, this, &RegexToolPlugin::clear);
        operationsLayout->addWidget(clearBtn);
        layout->addLayout(operationsLayout);

        QGroupBox* outputGroup = new QGroupBox(tr("结果"));
        QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
        m_outputEdit = new QTextEdit();
        m_outputEdit->setReadOnly(true);
        m_outputEdit->setPlaceholderText(tr("结果将显示在这里..."));
        outputLayout->addWidget(m_outputEdit);
        layout->addWidget(outputGroup);

        return widget;
    }

private slots:
    void findMatches()
    {
        QString pattern = m_regexEdit->text();
        QString text = m_inputEdit->toPlainText();
        if (pattern.isEmpty()) { m_outputEdit->setText(tr("请输入正则表达式")); return; }
        if (text.isEmpty()) { m_outputEdit->setText(tr("请输入测试文本")); return; }

        QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
        if (!m_caseSensitiveCb->isChecked()) options |= QRegularExpression::CaseInsensitiveOption;

        QRegularExpression regex(pattern, options);
        if (!regex.isValid()) { m_outputEdit->setText(tr("无效的正则表达式:\n%1").arg(regex.errorString())); return; }

        QRegularExpressionMatchIterator it = regex.globalMatch(text);
        QStringList matches;
        while (it.hasNext()) matches.append(it.next().captured());

        if (!matches.isEmpty()) {
            QString result = tr("找到 %1 个匹配:\n").arg(matches.size());
            for (int i = 0; i < matches.size(); ++i) result += QString("%1. %2\n").arg(i + 1).arg(matches[i]);
            m_outputEdit->setText(result);
        } else { m_outputEdit->setText(tr("未找到匹配")); }
    }

    void replaceMatches()
    {
        QString pattern = m_regexEdit->text();
        QString text = m_inputEdit->toPlainText();
        QString replacement = m_replaceEdit->text();
        if (pattern.isEmpty()) { m_outputEdit->setText(tr("请输入正则表达式")); return; }
        if (text.isEmpty()) { m_outputEdit->setText(tr("请输入测试文本")); return; }

        QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
        if (!m_caseSensitiveCb->isChecked()) options |= QRegularExpression::CaseInsensitiveOption;

        QRegularExpression regex(pattern, options);
        if (!regex.isValid()) { m_outputEdit->setText(tr("无效的正则表达式")); return; }

        QString result = text;
        result.replace(regex, replacement);
        m_outputEdit->setText(tr("替换完成:\n\n%1").arg(result));
    }

    void clear() { m_regexEdit->clear(); m_inputEdit->clear(); m_replaceEdit->clear(); m_outputEdit->clear(); }

private:
    QLineEdit* m_regexEdit = nullptr;
    QTextEdit* m_inputEdit = nullptr;
    QLineEdit* m_replaceEdit = nullptr;
    QTextEdit* m_outputEdit = nullptr;
    QCheckBox* m_caseSensitiveCb = nullptr;
};
