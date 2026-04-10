#ifndef HISTORY_DIALOG_H
#define HISTORY_DIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

class HistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget* parent = nullptr);

private slots:
    void onRefreshClicked();
    void onExportCsvClicked();

private:
    void loadTable();
    void setRow(int row, const QString& time, const QString& path, const QString& type,
                float conf, const QString& box);

    QTableWidget* table;
    QSpinBox* limitSpin;
    QPushButton* refreshButton;
    QPushButton* exportButton;
    QPushButton* closeButton;
};

#endif
