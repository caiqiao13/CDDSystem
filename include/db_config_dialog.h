#ifndef DB_CONFIG_DIALOG_H
#define DB_CONFIG_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

class DbConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit DbConfigDialog(QWidget *parent = nullptr);
    ~DbConfigDialog();

private slots:
    void onSaveClicked();
    void onTestConnectionClicked();

private:
    void loadConfig();

    QLineEdit *hostEdit;
    QLineEdit *portEdit;
    QLineEdit *dbNameEdit;
    QLineEdit *userEdit;
    QLineEdit *passwordEdit;
    QPushButton *saveButton;
    QPushButton *testButton;
    QPushButton *cancelButton;
};

#endif // DB_CONFIG_DIALOG_H
