#pragma once

#include "plugins/base.h"
#include <QDateTime>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QClipboard>

class TimestampToolPlugin : public BasePlugin
{
    Q_OBJECT

public:
    TimestampToolPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        setName(tr("时间戳工具"));
        setIcon("⏰");
        setDescription(tr("时间戳和日期时间转换"));
        setCategory("text");
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* currentGroup = new QGroupBox(tr("当前时间"));
        QHBoxLayout* currentLayout = new QHBoxLayout(currentGroup);
        m_currentLabel = new QLabel();
        QFont currentFont = m_currentLabel->font();
        currentFont.setPointSize(14);
        currentFont.setBold(true);
        m_currentLabel->setFont(currentFont);
        currentLayout->addWidget(m_currentLabel);
        currentLayout->addStretch();
        QPushButton* refreshBtn = new QPushButton(tr("🔄 刷新"));
        connect(refreshBtn, &QPushButton::clicked, this, &TimestampToolPlugin::updateCurrentTime);
        currentLayout->addWidget(refreshBtn);
        layout->addWidget(currentGroup);

        QGroupBox* convertGroup = new QGroupBox(tr("转换"));
        QVBoxLayout* convertLayout = new QVBoxLayout(convertGroup);
        QHBoxLayout* timestampLayout = new QHBoxLayout();
        timestampLayout->addWidget(new QLabel(tr("时间戳:")));
        m_timestampEdit = new QLineEdit();
        m_timestampEdit->setPlaceholderText(tr("输入时间戳 (秒或毫秒)..."));
        timestampLayout->addWidget(m_timestampEdit);
        QPushButton* tsToDateBtn = new QPushButton(tr("➡ 转为日期"));
        connect(tsToDateBtn, &QPushButton::clicked, this, &TimestampToolPlugin::timestampToDate);
        timestampLayout->addWidget(tsToDateBtn);
        convertLayout->addLayout(timestampLayout);

        QHBoxLayout* dateLayout = new QHBoxLayout();
        dateLayout->addWidget(new QLabel(tr("日期:")));
        m_dateEdit = new QLineEdit();
        m_dateEdit->setPlaceholderText(tr("格式: YYYY-MM-DD HH:mm:ss"));
        dateLayout->addWidget(m_dateEdit);
        QPushButton* dateToTsBtn = new QPushButton(tr("➡ 转为时间戳"));
        connect(dateToTsBtn, &QPushButton::clicked, this, &TimestampToolPlugin::dateToTimestamp);
        dateLayout->addWidget(dateToTsBtn);
        convertLayout->addLayout(dateLayout);
        layout->addWidget(convertGroup);

        QGroupBox* resultGroup = new QGroupBox(tr("结果"));
        QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);
        m_resultEdit = new QLineEdit();
        m_resultEdit->setReadOnly(true);
        m_resultEdit->setPlaceholderText(tr("结果将显示在这里..."));
        resultLayout->addWidget(m_resultEdit);
        QPushButton* copyBtn = new QPushButton(tr("📋 复制结果"));
        connect(copyBtn, &QPushButton::clicked, this, &TimestampToolPlugin::copyResult);
        resultLayout->addWidget(copyBtn);
        layout->addWidget(resultGroup);

        updateCurrentTime();
        return widget;
    }

private slots:
    void updateCurrentTime()
    {
        QDateTime now = QDateTime::currentDateTime();
        m_currentLabel->setText(tr("日期: %1 | 秒: %2 | 毫秒: %3")
            .arg(now.toString("yyyy-MM-dd HH:mm:ss"))
            .arg(now.toSecsSinceEpoch())
            .arg(now.toMSecsSinceEpoch()));
    }

    void timestampToDate()
    {
        QString text = m_timestampEdit->text().trimmed();
        if (text.isEmpty()) { m_resultEdit->setText(tr("请输入时间戳")); return; }

        bool ok;
        qint64 timestamp = text.toLongLong(&ok);
        if (!ok) { m_resultEdit->setText(tr("无效的时间戳")); return; }

        QDateTime dateTime;
        if (timestamp > 100000000000LL) dateTime = QDateTime::fromMSecsSinceEpoch(timestamp);
        else dateTime = QDateTime::fromSecsSinceEpoch(timestamp);

        if (!dateTime.isValid()) { m_resultEdit->setText(tr("无效的时间戳")); return; }

        m_resultEdit->setText(tr("日期时间: %1").arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
    }

    void dateToTimestamp()
    {
        QString text = m_dateEdit->text().trimmed();
        if (text.isEmpty()) { m_resultEdit->setText(tr("请输入日期")); return; }

        QDateTime dateTime = QDateTime::fromString(text, "yyyy-MM-dd HH:mm:ss");
        if (!dateTime.isValid()) dateTime = QDateTime::fromString(text, "yyyy/MM/dd HH:mm:ss");

        if (!dateTime.isValid()) { m_resultEdit->setText(tr("无效的日期格式")); return; }

        m_resultEdit->setText(tr("秒级时间戳: %1").arg(dateTime.toSecsSinceEpoch()));
    }

    void copyResult() { QApplication::clipboard()->setText(m_resultEdit->text()); }

private:
    QLabel* m_currentLabel = nullptr;
    QLineEdit* m_timestampEdit = nullptr;
    QLineEdit* m_dateEdit = nullptr;
    QLineEdit* m_resultEdit = nullptr;
};
