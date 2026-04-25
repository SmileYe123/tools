#pragma once

#include "plugins/base.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QCryptographicHash>

class HashToolPlugin : public BasePlugin
{
    Q_OBJECT

public:
    HashToolPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        setName(tr("哈希计算"));
        setIcon("🔐");
        setDescription(tr("计算MD5、SHA1、SHA256哈希值"));
        setCategory("security");
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* inputGroup = new QGroupBox(tr("输入"));
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
        m_textInput = new QTextEdit();
        m_textInput->setPlaceholderText(tr("输入要计算哈希的文本..."));
        inputLayout->addWidget(m_textInput);
        layout->addWidget(inputGroup);

        QHBoxLayout* algorithmLayout = new QHBoxLayout();
        algorithmLayout->addWidget(new QLabel(tr("算法:")));
        m_algorithmCombo = new QComboBox();
        m_algorithmCombo->addItems({"MD5", "SHA1", "SHA256", "SHA512"});
        algorithmLayout->addWidget(m_algorithmCombo);
        algorithmLayout->addStretch();
        layout->addLayout(algorithmLayout);

        QPushButton* calculateBtn = new QPushButton(tr("🔢 计算哈希"));
        connect(calculateBtn, &QPushButton::clicked, this, &HashToolPlugin::calculateHash);
        layout->addWidget(calculateBtn);

        QGroupBox* outputGroup = new QGroupBox(tr("哈希结果"));
        QVBoxLayout* outputLayout = new QVBoxLayout(outputGroup);
        QHBoxLayout* resultLayout = new QHBoxLayout();
        m_resultEdit = new QLineEdit();
        m_resultEdit->setReadOnly(true);
        m_resultEdit->setPlaceholderText(tr("哈希结果..."));
        resultLayout->addWidget(m_resultEdit);
        QPushButton* copyBtn = new QPushButton(tr("📋 复制"));
        connect(copyBtn, &QPushButton::clicked, this, &HashToolPlugin::copyResult);
        resultLayout->addWidget(copyBtn);
        outputLayout->addLayout(resultLayout);
        layout->addWidget(outputGroup);

        return widget;
    }

private slots:
    void calculateHash()
    {
        QByteArray data = m_textInput->toPlainText().toUtf8();
        if (data.isEmpty()) { m_resultEdit->setText(tr("没有输入数据")); return; }

        QCryptographicHash::Algorithm algorithm;
        QString algoName = m_algorithmCombo->currentText();
        if (algoName == "MD5") algorithm = QCryptographicHash::Md5;
        else if (algoName == "SHA1") algorithm = QCryptographicHash::Sha1;
        else if (algoName == "SHA256") algorithm = QCryptographicHash::Sha256;
        else algorithm = QCryptographicHash::Sha512;

        m_resultEdit->setText(QCryptographicHash::hash(data, algorithm).toHex().toUpper());
    }

    void copyResult() { QApplication::clipboard()->setText(m_resultEdit->text()); }

private:
    QTextEdit* m_textInput = nullptr;
    QComboBox* m_algorithmCombo = nullptr;
    QLineEdit* m_resultEdit = nullptr;
};
