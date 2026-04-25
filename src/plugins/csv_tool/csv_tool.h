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
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

class CsvToolPlugin : public BasePlugin
{
    Q_OBJECT

public:
    CsvToolPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        setName(tr("CSV工具"));
        setIcon("📊");
        setDescription(tr("CSV文件处理和转换"));
        setCategory("data");
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        QWidget* widget = new QWidget(parent);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(12);

        QGroupBox* fileGroup = new QGroupBox(tr("CSV文件"));
        QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup);

        QHBoxLayout* pathLayout = new QHBoxLayout();
        m_fileEdit = new QLineEdit();
        m_fileEdit->setReadOnly(true);
        m_fileEdit->setPlaceholderText(tr("选择一个CSV文件..."));
        pathLayout->addWidget(m_fileEdit);

        QPushButton* browseBtn = new QPushButton(tr("浏览..."));
        connect(browseBtn, &QPushButton::clicked, this, &CsvToolPlugin::browseFile);
        pathLayout->addWidget(browseBtn);

        QPushButton* loadBtn = new QPushButton(tr("加载"));
        connect(loadBtn, &QPushButton::clicked, this, &CsvToolPlugin::loadCsv);
        pathLayout->addWidget(loadBtn);

        fileLayout->addLayout(pathLayout);
        layout->addWidget(fileGroup);

        QGroupBox* settingsGroup = new QGroupBox(tr("设置"));
        QHBoxLayout* settingsLayout = new QHBoxLayout(settingsGroup);
        settingsLayout->addWidget(new QLabel(tr("分隔符:")));
        m_delimiterCombo = new QComboBox();
        m_delimiterCombo->addItems({",", ";", "\t", "|"});
        settingsLayout->addWidget(m_delimiterCombo);
        settingsLayout->addStretch();
        layout->addWidget(settingsGroup);

        QGroupBox* previewGroup = new QGroupBox(tr("预览"));
        QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
        m_tableWidget = new QTableWidget();
        m_tableWidget->setAlternatingRowColors(true);
        previewLayout->addWidget(m_tableWidget);
        layout->addWidget(previewGroup);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* toJsonBtn = new QPushButton(tr("📄 转为JSON"));
        connect(toJsonBtn, &QPushButton::clicked, this, &CsvToolPlugin::convertToJson);
        buttonLayout->addWidget(toJsonBtn);

        QPushButton* clearBtn = new QPushButton(tr("🗑️ 清空"));
        connect(clearBtn, &QPushButton::clicked, this, &CsvToolPlugin::clear);
        buttonLayout->addWidget(clearBtn);
        layout->addLayout(buttonLayout);

        QGroupBox* resultGroup = new QGroupBox(tr("结果"));
        QVBoxLayout* resultLayout = new QVBoxLayout(resultGroup);
        m_resultEdit = new QTextEdit();
        m_resultEdit->setReadOnly(true);
        m_resultEdit->setPlaceholderText(tr("结果将显示在这里..."));
        resultLayout->addWidget(m_resultEdit);
        layout->addWidget(resultGroup);

        return widget;
    }

private slots:
    void browseFile()
    {
        QString fileName = QFileDialog::getOpenFileName(nullptr, tr("选择CSV文件"), QString(), tr("CSV文件 (*.csv *.txt);;所有文件 (*)"));
        if (!fileName.isEmpty()) m_fileEdit->setText(fileName);
    }

    void loadCsv()
    {
        QString filePath = m_fileEdit->text();
        if (filePath.isEmpty()) { QMessageBox::warning(nullptr, tr("警告"), tr("请选择一个CSV文件")); return; }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { QMessageBox::warning(nullptr, tr("警告"), tr("打开文件失败")); return; }

        m_csvData.clear();
        QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif
        while (!in.atEnd()) m_csvData.append(in.readLine());
        file.close();
        updateTable();
    }

    void convertToJson()
    {
        if (m_csvData.isEmpty()) { m_resultEdit->setText(tr("没有加载数据")); return; }
        QString delimiter = m_delimiterCombo->currentText();
        QStringList headers = parseCsvLine(m_csvData[0], delimiter);
        QString json = "[\n";
        for (int i = 1; i < m_csvData.size(); ++i) {
            QStringList values = parseCsvLine(m_csvData[i], delimiter);
            json += "  {\n";
            for (int j = 0; j < headers.size() && j < values.size(); ++j) {
                json += QString("    \"%1\": \"%2\"").arg(headers[j]).arg(values[j]);
                if (j < headers.size() - 1 && j < values.size() - 1) json += ",";
                json += "\n";
            }
            json += "  }";
            if (i < m_csvData.size() - 1) json += ",";
            json += "\n";
        }
        json += "]";
        m_resultEdit->setText(json);
    }

    void clear() { m_fileEdit->clear(); m_csvData.clear(); m_tableWidget->clear(); m_resultEdit->clear(); }

private:
    void updateTable()
    {
        if (m_csvData.isEmpty()) return;
        QString delimiter = m_delimiterCombo->currentText();
        QStringList headers = parseCsvLine(m_csvData[0], delimiter);
        m_tableWidget->clear();
        m_tableWidget->setColumnCount(headers.size());
        m_tableWidget->setHorizontalHeaderLabels(headers);
        m_tableWidget->setRowCount(m_csvData.size() - 1);
        for (int i = 1; i < m_csvData.size(); ++i) {
            QStringList values = parseCsvLine(m_csvData[i], delimiter);
            for (int j = 0; j < values.size(); ++j)
                m_tableWidget->setItem(i - 1, j, new QTableWidgetItem(values[j]));
        }
        m_tableWidget->resizeColumnsToContents();
    }

    QStringList parseCsvLine(const QString& line, const QString& delimiter)
    {
        QStringList result;
        QString current;
        bool inQuotes = false;
        for (int i = 0; i < line.length(); ++i) {
            QChar c = line[i];
            if (c == '"') { inQuotes = !inQuotes; }
            else if (!inQuotes && line.mid(i, delimiter.length()) == delimiter) {
                result.append(current); current.clear(); i += delimiter.length() - 1;
            } else { current += c; }
        }
        result.append(current);
        return result;
    }

    QLineEdit* m_fileEdit = nullptr;
    QComboBox* m_delimiterCombo = nullptr;
    QTableWidget* m_tableWidget = nullptr;
    QTextEdit* m_resultEdit = nullptr;
    QStringList m_csvData;
};
