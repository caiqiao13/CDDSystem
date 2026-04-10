#include "db_manager.h"
#include <QDebug>
#include <QDateTime>

DBManager::DBManager() {
    if (!QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::addDatabase("QMYSQL");
    } else {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
}

DBManager::~DBManager() {
    closeDatabase();
}

bool DBManager::connectToDatabase(const QString& host, const QString& dbName, const QString& user, const QString& password, int port) {
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);

    if (!db.open()) {
        qDebug() << "Error: Failed to connect to database." << db.lastError().text();
        return false;
    }

    // 自动创建表
    QSqlQuery query(db);
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS detection_logs (
            id INT AUTO_INCREMENT PRIMARY KEY,
            image_path VARCHAR(255) NOT NULL,
            defect_type VARCHAR(100) NOT NULL,
            confidence FLOAT NOT NULL,
            bounding_box VARCHAR(100) NOT NULL,
            detection_time DATETIME NOT NULL
        )
    )";
    
    if (!query.exec(createTableQuery)) {
        qDebug() << "Error: Failed to create table." << query.lastError().text();
        return false;
    }

    return true;
}

bool DBManager::insertRecord(const DetectionRecord& record) {
    if (!db.isOpen()) {
        qDebug() << "Error: Database is not open.";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO detection_logs (image_path, defect_type, confidence, bounding_box, detection_time) "
                  "VALUES (:image_path, :defect_type, :confidence, :bounding_box, :detection_time)");
    
    query.bindValue(":image_path", record.image_path);
    query.bindValue(":defect_type", record.defect_type);
    query.bindValue(":confidence", record.confidence);
    query.bindValue(":bounding_box", record.bounding_box);
    query.bindValue(":detection_time", record.detection_time.isEmpty() ? QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") : record.detection_time);

    if (!query.exec()) {
        qDebug() << "Error: Failed to insert record." << query.lastError().text();
        return false;
    }
    return true;
}

QList<DetectionRecord> DBManager::fetchRecentRecords(int limit) {
    QList<DetectionRecord> list;
    if (!db.isOpen()) {
        return list;
    }
    int lim = limit;
    if (lim < 1) lim = 1;
    if (lim > 10000) lim = 10000;
    QSqlQuery query(db);
    const QString sql = QString(
        "SELECT image_path, defect_type, confidence, bounding_box, detection_time "
        "FROM detection_logs ORDER BY id DESC LIMIT %1"
    ).arg(lim);
    if (!query.exec(sql)) {
        qDebug() << "Error: Failed to fetch records." << query.lastError().text();
        return list;
    }
    while (query.next()) {
        DetectionRecord r;
        r.image_path = query.value(0).toString();
        r.defect_type = query.value(1).toString();
        r.confidence = query.value(2).toFloat();
        r.bounding_box = query.value(3).toString();
        r.detection_time = query.value(4).toString();
        list.append(r);
    }
    return list;
}

void DBManager::closeDatabase() {
    if (db.isOpen()) {
        db.close();
    }
}
