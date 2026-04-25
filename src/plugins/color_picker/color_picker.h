#pragma once

#include "plugins/base.h"
#include <QApplication>
#include <QColorDialog>
#include <QClipboard>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScreen>
#include <QTimer>
#include <QRegularExpression>
#include <QPainter>

class ColorPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    ColorPreviewWidget(QWidget* parent = nullptr) : QWidget(parent), m_color(Qt::black) {
        setMinimumSize(100, 60);
        setMaximumHeight(80);
    }
    void setColor(const QColor& color) { m_color = color; update(); }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.fillRect(rect(), m_color);
    }
private:
    QColor m_color;
};

class ColorPickerPlugin : public BasePlugin
{
    Q_OBJECT

public:
    explicit ColorPickerPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        setName(tr("颜色选择"));
        setIcon("🎨");
        setDescription(tr("屏幕取色和颜色格式转换"));
        setCategory("ui");
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* inputGroup = new QGroupBox(tr("颜色输入"));
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);

        QHBoxLayout* inputComboLayout = new QHBoxLayout();
        m_formatCombo = new QComboBox();
        m_formatCombo->addItems({tr("HEX"), tr("RGB"), tr("HSL")});
        inputComboLayout->addWidget(m_formatCombo);

        m_inputEdit = new QLineEdit();
        m_inputEdit->setPlaceholderText(tr("输入颜色值 (例如: #FF5733)"));
        inputComboLayout->addWidget(m_inputEdit);

        m_convertButton = new QPushButton(tr("转换"));
        connect(m_convertButton, &QPushButton::clicked, this, &ColorPickerPlugin::convertColor);
        inputComboLayout->addWidget(m_convertButton);

        inputLayout->addLayout(inputComboLayout);
        layout->addWidget(inputGroup);

        QGroupBox* previewGroup = new QGroupBox(tr("颜色预览"));
        QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
        m_colorPreview = new ColorPreviewWidget();
        previewLayout->addWidget(m_colorPreview);
        layout->addWidget(previewGroup);

        QGroupBox* colorGroup = new QGroupBox(tr("颜色值"));
        QGridLayout* colorLayout = new QGridLayout(colorGroup);
        colorLayout->setSpacing(10);

        colorLayout->addWidget(new QLabel(tr("HEX:")), 0, 0);
        m_hexInput = new QLineEdit();
        m_hexInput->setReadOnly(true);
        colorLayout->addWidget(m_hexInput, 0, 1);

        colorLayout->addWidget(new QLabel(tr("RGB:")), 1, 0);
        m_rgbInput = new QLineEdit();
        m_rgbInput->setReadOnly(true);
        colorLayout->addWidget(m_rgbInput, 1, 1);

        colorLayout->addWidget(new QLabel(tr("HSL:")), 2, 0);
        m_hslInput = new QLineEdit();
        m_hslInput->setReadOnly(true);
        colorLayout->addWidget(m_hslInput, 2, 1);

        layout->addWidget(colorGroup);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(8);

        m_pickButton = new QPushButton(tr("🎨 选择颜色"));
        connect(m_pickButton, &QPushButton::clicked, this, &ColorPickerPlugin::pickColor);
        buttonLayout->addWidget(m_pickButton);

        m_copyHexButton = new QPushButton(tr("📋 复制HEX"));
        connect(m_copyHexButton, &QPushButton::clicked, this, &ColorPickerPlugin::copyHex);
        buttonLayout->addWidget(m_copyHexButton);

        m_copyRgbButton = new QPushButton(tr("📋 复制RGB"));
        connect(m_copyRgbButton, &QPushButton::clicked, this, &ColorPickerPlugin::copyRgb);
        buttonLayout->addWidget(m_copyRgbButton);

        layout->addLayout(buttonLayout);

        return widget;
    }

private slots:
    void pickColor()
    {
        QColor color = QColorDialog::getColor(m_currentColor, nullptr, tr("选择颜色"));
        if (color.isValid()) {
            setColor(color);
        }
    }

    void convertColor()
    {
        QString input = m_inputEdit->text().trimmed();
        if (input.isEmpty()) return;

        QColor color;
        QString format = m_formatCombo->currentText();

        if (format == tr("HEX")) {
            color = QColor(input);
        } else if (format == tr("RGB")) {
            QRegularExpression rgbRegex("RGB\\s*\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\)", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = rgbRegex.match(input);
            if (match.hasMatch()) {
                color = QColor(match.captured(1).toInt(), match.captured(2).toInt(), match.captured(3).toInt());
            }
        }

        if (color.isValid()) {
            setColor(color);
        }
    }

    void copyHex() { QApplication::clipboard()->setText(m_hexInput->text()); }
    void copyRgb() { QApplication::clipboard()->setText(m_rgbInput->text()); }

private:
    void setColor(const QColor& color)
    {
        m_currentColor = color;
        m_colorPreview->setColor(color);
        m_hexInput->setText(color.name().toUpper());

        m_rgbInput->setText(QString("RGB(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue()));

        int h = color.hslHue();
        int s = color.hslSaturation();
        int l = color.lightness();
        QString hStr = (h == -1) ? "0" : QString::number(h);
        int sPercent = s * 100 / 255;
        int lPercent = l * 100 / 255;
        m_hslInput->setText(QString("HSL(%1, %2%, %3%)").arg(hStr).arg(sPercent).arg(lPercent));
    }

    QComboBox* m_formatCombo = nullptr;
    QLineEdit* m_inputEdit = nullptr;
    QPushButton* m_convertButton = nullptr;
    QPushButton* m_pickButton = nullptr;
    QPushButton* m_copyHexButton = nullptr;
    QPushButton* m_copyRgbButton = nullptr;
    ColorPreviewWidget* m_colorPreview = nullptr;
    QLineEdit* m_hexInput = nullptr;
    QLineEdit* m_rgbInput = nullptr;
    QLineEdit* m_hslInput = nullptr;
    QColor m_currentColor = Qt::black;
};
