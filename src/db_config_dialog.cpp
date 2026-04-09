#include "db_config_dialog.h"
#include "db_manager.h"
#include <QSettings>
#include <QMessageBox>
#include <QLabel>

DbConfigDialog::DbConfigDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("MySQL 数据库配置");
    setFixedSize(300, 250);

    hostEdit = new QLineEdit(this);
    portEdit = new QLineEdit(this);
    dbNameEdit = new QLineEdit(this);
    userEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);

    saveButton = new QPushButton("保存", this);
    testButton = new QPushButton("测试连接", this);
    cancelButton = new QPushButton("取消", this);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(new QLabel("主机 (Host):"), hostEdit);
    formLayout->addRow(new QLabel("端口 (Port):"), portEdit);
    formLayout->addRow(new QLabel("数据库 (Database):"), dbNameEdit);
    formLayout->addRow(new QLabel("用户名 (User):"), userEdit);
    formLayout->addRow(new QLabel("密码 (Password):"), passwordEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(testButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);

    connect(saveButton, &QPushButton::clicked, this, &DbConfigDialog::onSaveClicked);
    connect(testButton, &QPushButton::clicked, this, &DbConfigDialog::onTestConnectionClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    loadConfig();
}

DbConfigDialog::~DbConfigDialog() {}

void DbConfigDialog::loadConfig() {
    QSettings settings("config.ini", QSettings::IniFormat);
    hostEdit->setText(settings.value("Database/Host", "127.0.0.1").toString());
    portEdit->setText(settings.value("Database/Port", "3306").toString());
    dbNameEdit->setText(settings.value("Database/Name", "defect_db").toString());
    userEdit->setText(settings.value("Database/User", "root").toString());
    passwordEdit->setText(settings.value("Database/Password", "root").toString());
}

void DbConfigDialog::onSaveClicked() {
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.setValue("Database/Host", hostEdit->text());
    settings.setValue("Database/Port", portEdit->text());
    settings.setValue("Database/Name", dbNameEdit->text());
    settings.setValue("Database/User", userEdit->text());
    settings.setValue("Database/Password", passwordEdit->text());
    
    QMessageBox::information(this, "成功", "数据库配置已保存！\n需要重启系统或点击“测试连接”应用。");
    accept();
}

void DbConfigDialog::onTestConnectionClicked() {
    QString host = hostEdit->text();
    int port = portEdit->text().toInt();
    QString dbName = dbNameEdit->text();
    QString user = userEdit->text();
    QString password = passwordEdit->text();

    DBManager::getInstance().closeDatabase();
    if (DBManager::getInstance().connectToDatabase(host, dbName, user, password, port)) {
        QMessageBox::information(this, "成功", "数据库连接成功！");
    } else {
        QMessageBox::critical(this, "失败", "数据库连接失败，请检查配置信息或 MySQL 服务状态。");
    }
}
