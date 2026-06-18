#include "datacontroller.h"
#include "../model/database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

DataController::DataController(QObject *parent) : QObject(parent) {}

DataController& DataController::instance() {
    static DataController inst;
    return inst;
}

// =========================================================================
// Branch CRUD
// =========================================================================

QList<Branch> DataController::getAllBranches() {
    QList<Branch> list;
    if (!Database::instance().isOpen()) return list;

    QSqlQuery q("SELECT * FROM branch ORDER BY id ASC", Database::instance().db());
    while (q.next()) {
        list.append(Branch::fromQuery(q));
    }
    return list;
}

bool DataController::addBranch(const QString& name, const QString& address) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("INSERT INTO branch (name, address) VALUES (:name, :address)");
    q.bindValue(":name", name);
    q.bindValue(":address", address);
    return q.exec();
}

bool DataController::updateBranch(int id, const QString& name, const QString& address) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("UPDATE branch SET name = :name, address = :address WHERE id = :id");
    q.bindValue(":name", name);
    q.bindValue(":address", address);
    q.bindValue(":id", id);
    return q.exec();
}

bool DataController::deleteBranch(int id, QString& errorMsg) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("DELETE FROM branch WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        errorMsg = "Не удалось удалить филиал. Возможно, к нему привязаны заказы.";
        qDebug() << "Delete branch failed:" << q.lastError().text();
        return false;
    }
    return true;
}

