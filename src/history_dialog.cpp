#include "history_dialog.h"
#include "db_manager.h"
#include <QAbstractItemView>
#include <QHeaderView>
#include <QFileDialog>
#include <QTextStream>
#include <QStringConverter>
#include <QMessageBox>
#include <QTableWidgetItem>

HistoryDialog::HistoryDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("检测记录");
    resize(900, 480);

    auto* top = new QHBoxLayout();
    top->addWidget(new QLabel("最大条数:", this));
    limitSpin = new QSpinBox(this);
    limitSpin->setRange(50, 10000);
    limitSpin->setValue(500);
    limitSpin->setSingleStep(50);
    top->addWidget(limitSpin);
    refreshButton = new QPushButton("刷新", this);
    exportButton = new QPushButton("导出 CSV", this);
    closeButton = new QPushButton("关闭", this);
    top->addWidget(refreshButton);
    top->addWidget(exportButton);
    top->addStretch();
    top->addWidget(closeButton);

    table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels(QStringList()
        << "检测时间" << "图片路径" << "缺陷类型" << "置信度" << "边界框(x,y,w,h)");
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto* main = new QVBoxLayout(this);
    main->addLayout(top);
    main->addWidget(table);

    connect(refreshButton, &QPushButton::clicked, this, &HistoryDialog::onRefreshClicked);
    connect(exportButton, &QPushButton::clicked, this, &HistoryDialog::onExportCsvClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    loadTable();
}

void HistoryDialog::setRow(int row, const QString& time, const QString& path, const QString& type,
                           float conf, const QString& box)
{
    table->setItem(row, 0, new QTableWidgetItem(time));
    table->setItem(row, 1, new QTableWidgetItem(path));
    table->setItem(row, 2, new QTableWidgetItem(type));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(conf, 'f', 4)));
    table->setItem(row, 4, new QTableWidgetItem(box));
}

void HistoryDialog::loadTable()
{
    QList<DetectionRecord> rows = DBManager::getInstance().fetchRecentRecords(limitSpin->value());
    table->setRowCount(rows.size());
    for (int i = 0; i < rows.size(); ++i) {
        const DetectionRecord& r = rows[i];
        setRow(i, r.detection_time, r.image_path, r.defect_type, r.confidence, r.bounding_box);
    }
    table->resizeColumnsToContents();
}

void HistoryDialog::onRefreshClicked()
{
    loadTable();
}

void HistoryDialog::onExportCsvClicked()
{
    QString path = QFileDialog::getSaveFileName(this, "导出 CSV", "detection_logs.csv", "CSV (*.csv)");
    if (path.isEmpty()) {
        return;
    }
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法写入文件");
        return;
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << "detection_time,image_path,defect_type,confidence,bounding_box\n";
    QList<DetectionRecord> rows = DBManager::getInstance().fetchRecentRecords(limitSpin->value());
    for (const DetectionRecord& r : rows) {
        auto esc = [](const QString& s) -> QString {
            QString t = s;
            t.replace('"', "\"\"");
            if (t.contains(',') || t.contains('"') || t.contains('\n')) {
                return QString("\"%1\"").arg(t);
            }
            return t;
        };
        out << esc(r.detection_time) << ','
            << esc(r.image_path) << ','
            << esc(r.defect_type) << ','
            << QString::number(r.confidence, 'f', 6) << ','
            << esc(r.bounding_box) << '\n';
    }
    QMessageBox::information(this, "完成", QString("已导出 %1 条记录").arg(rows.size()));
}
