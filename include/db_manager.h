#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

struct DetectionRecord {
    QString image_path;
    QString defect_type;
    float confidence;
    QString bounding_box;
    QString detection_time;
};

class DBManager {
public:
    static DBManager& getInstance() {
        static DBManager instance;
        return instance;
    }

    bool connectToDatabase(const QString& host, const QString& dbName, const QString& user, const QString& password, int port = 3306);
    bool insertRecord(const DetectionRecord& record);
    void closeDatabase();

private:
    DBManager();
    ~DBManager();
    QSqlDatabase db;
};

#endif // DB_MANAGER_H
