#pragma once

#include "plugins/base.h"
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QProgressBar>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>

class ImageToolPlugin : public BasePlugin
{
    Q_OBJECT

public:
    ImageToolPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        m_name = tr("图片工具");
        m_icon = "🖼️";
        m_description = tr("图片压缩和格式转换");
        m_category = "media";
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* inputGroup = new QGroupBox(tr("选择图片"));
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
        QHBoxLayout* fileLayout = new QHBoxLayout();
        m_fileEdit = new QLineEdit();
        m_fileEdit->setReadOnly(true);
        m_fileEdit->setPlaceholderText(tr("选择一个图片文件..."));
        fileLayout->addWidget(m_fileEdit);
        QPushButton* browseBtn = new QPushButton(tr("浏览..."));
        connect(browseBtn, &QPushButton::clicked, this, &ImageToolPlugin::browseImage);
        fileLayout->addWidget(browseBtn);
        inputLayout->addLayout(fileLayout);
        QHBoxLayout* infoLayout = new QHBoxLayout();
        m_infoLabel = new QLabel(tr("未选择图片"));
        infoLayout->addWidget(m_infoLabel);
        infoLayout->addStretch();
        inputLayout->addLayout(infoLayout);
        layout->addWidget(inputGroup);

        QGroupBox* settingsGroup = new QGroupBox(tr("设置"));
        QVBoxLayout* settingsLayout = new QVBoxLayout(settingsGroup);
        QHBoxLayout* formatLayout = new QHBoxLayout();
        formatLayout->addWidget(new QLabel(tr("输出格式:")));
        m_formatCombo = new QComboBox();
        m_formatCombo->addItems({"PNG", "JPG", "WEBP", "BMP"});
        formatLayout->addWidget(m_formatCombo);
        formatLayout->addStretch();
        settingsLayout->addLayout(formatLayout);
        QHBoxLayout* qualityLayout = new QHBoxLayout();
        qualityLayout->addWidget(new QLabel(tr("质量:")));
        m_qualitySlider = new QSlider(Qt::Horizontal);
        m_qualitySlider->setRange(1, 100);
        m_qualitySlider->setValue(85);
        connect(m_qualitySlider, &QSlider::valueChanged, this, [this](int v) { m_qualityLabel->setText(QString("%1%").arg(v)); });
        qualityLayout->addWidget(m_qualitySlider);
        m_qualityLabel = new QLabel("85%");
        qualityLayout->addWidget(m_qualityLabel);
        settingsLayout->addLayout(qualityLayout);
        layout->addWidget(settingsGroup);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* convertBtn = new QPushButton(tr("🔄 转换"));
        connect(convertBtn, &QPushButton::clicked, this, &ImageToolPlugin::convertImage);
        buttonLayout->addWidget(convertBtn);
        layout->addLayout(buttonLayout);

        QGroupBox* resultGroup = new QGroupBox(tr("结果"));
        QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);
        m_resultEdit = new QLineEdit();
        m_resultEdit->setReadOnly(true);
        m_resultEdit->setPlaceholderText(tr("结果将显示在这里..."));
        resultLayout->addWidget(m_resultEdit);
        layout->addWidget(resultGroup);

        return widget;
    }

private slots:
    void browseImage()
    {
        QString fileName = QFileDialog::getOpenFileName(nullptr, tr("选择图片"), QString(), tr("图片 (*.png *.jpg *.jpeg *.bmp *.webp)"));
        if (!fileName.isEmpty()) { m_fileEdit->setText(fileName); updateImageInfo(fileName); }
    }

    void convertImage()
    {
        QString inputPath = m_fileEdit->text();
        if (inputPath.isEmpty()) { m_resultEdit->setText(tr("请选择一个图片")); return; }

        QImage image(inputPath);
        if (image.isNull()) { m_resultEdit->setText(tr("加载图片失败")); return; }

        QString format = m_formatCombo->currentText().toLower();
        QString outputPath = QFileDialog::getSaveFileName(nullptr, tr("保存图片"), QString(), tr("%1文件 (*.%2)").arg(format.toUpper()).arg(format));
        if (outputPath.isEmpty()) return;

        int quality = m_qualitySlider->value();
        if (image.save(outputPath, format.toUtf8().constData(), quality)) {
            QFileInfo info(outputPath);
            m_resultEdit->setText(tr("转换成功! 大小: %1 KB").arg(info.size() / 1024.0, 0, 'f', 2));
        } else {
            m_resultEdit->setText(tr("转换图片失败"));
        }
    }

private:
    void updateImageInfo(const QString& path)
    {
        QImageReader reader(path);
        QSize size = reader.size();
        QFileInfo info(path);
        m_infoLabel->setText(tr("尺寸: %1x%2 | 文件: %3 KB").arg(size.width()).arg(size.height()).arg(info.size() / 1024.0, 0, 'f', 2));
    }

    QLineEdit* m_fileEdit = nullptr;
    QLabel* m_infoLabel = nullptr;
    QComboBox* m_formatCombo = nullptr;
    QSlider* m_qualitySlider = nullptr;
    QLabel* m_qualityLabel = nullptr;
    QLineEdit* m_resultEdit = nullptr;
};
