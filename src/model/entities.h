#ifndef ENTITIES_H
#define ENTITIES_H

#include <QString>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

class User {
public:
    int id = 0;
    QString username;
    QString passwordHash;
    QString role; // "Admin" or "Employee"
    QString fullName;

    static User fromQuery(const QSqlQuery& q) {
        User u;
        u.id = q.value("id").toInt();
        u.username = q.value("username").toString();
        u.passwordHash = q.value("password_hash").toString();
        u.role = q.value("role").toString();
        u.fullName = q.value("full_name").toString();
        return u;
    }
};

