#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

class Database {
public:
    static Database& instance();
    
    bool connect(const QString& host, int port, const QString& dbName, const QString& user, const QString& password);
    void disconnect();
    bool isOpen() const;
    QSqlDatabase db() const;

private:
    Database() = default;
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    QSqlDatabase m_db;
};

#endif // DATABASE_H
