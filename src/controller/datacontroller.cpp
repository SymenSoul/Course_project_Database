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

// =========================================================================
// ServiceType CRUD
// =========================================================================

QList<ServiceType> DataController::getAllServiceTypes() {
    QList<ServiceType> list;
    if (!Database::instance().isOpen()) return list;

    QSqlQuery q("SELECT * FROM service_type ORDER BY id ASC", Database::instance().db());
    while (q.next()) {
        list.append(ServiceType::fromQuery(q));
    }
    return list;
}

bool DataController::addServiceType(const QString& name, const QString& type, double baseCost) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("INSERT INTO service_type (name, type, base_cost) VALUES (:name, :type, :base_cost)");
    q.bindValue(":name", name);
    q.bindValue(":type", type);
    q.bindValue(":base_cost", baseCost);
    return q.exec();
}

bool DataController::updateServiceType(int id, const QString& name, const QString& type, double baseCost) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("UPDATE service_type SET name = :name, type = :type, base_cost = :base_cost WHERE id = :id");
    q.bindValue(":name", name);
    q.bindValue(":type", type);
    q.bindValue(":base_cost", baseCost);
    q.bindValue(":id", id);
    return q.exec();
}

bool DataController::deleteServiceType(int id, QString& errorMsg) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("DELETE FROM service_type WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        errorMsg = "Не удалось удалить вид услуги. Возможно, он используется в оформленных заказах.";
        qDebug() << "Delete service type failed:" << q.lastError().text();
        return false;
    }
    return true;
}

// =========================================================================
// Client CRUD
// =========================================================================

QList<Client> DataController::getAllClients() {
    QList<Client> list;
    if (!Database::instance().isOpen()) return list;

    QSqlQuery q("SELECT * FROM client ORDER BY last_name, first_name ASC", Database::instance().db());
    while (q.next()) {
        list.append(Client::fromQuery(q));
    }
    return list;
}

bool DataController::addClient(const QString& lastName, const QString& firstName, const QString& middleName, const QString& phone) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("INSERT INTO client (last_name, first_name, middle_name, phone) VALUES (:last_name, :first_name, :middle_name, :phone)");
    q.bindValue(":last_name", lastName);
    q.bindValue(":first_name", firstName);
    q.bindValue(":middle_name", middleName.trimmed().isEmpty() ? QVariant(QVariant::String) : middleName);
    q.bindValue(":phone", phone);
    return q.exec();
}

bool DataController::updateClient(int id, const QString& lastName, const QString& firstName, const QString& middleName, const QString& phone) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("UPDATE client SET last_name = :last_name, first_name = :first_name, middle_name = :middle_name, phone = :phone WHERE id = :id");
    q.bindValue(":last_name", lastName);
    q.bindValue(":first_name", firstName);
    q.bindValue(":middle_name", middleName.trimmed().isEmpty() ? QVariant(QVariant::String) : middleName);
    q.bindValue(":phone", phone);
    q.bindValue(":id", id);
    return q.exec();
}

bool DataController::deleteClient(int id, QString& errorMsg) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("DELETE FROM client WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        errorMsg = "Не удалось удалить клиента. Возможно, у него есть зарегистрированные заказы.";
        qDebug() << "Delete client failed:" << q.lastError().text();
        return false;
    }
    return true;
}

int DataController::getClientOrderCount(int clientId) {
    if (!Database::instance().isOpen()) return 0;

    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT COUNT(*) FROM orders WHERE client_id = :client_id");
    q.bindValue(":client_id", clientId);
    if (q.exec() && q.next()) {
        return q.value(0).toInt();
    }
    return 0;
}

bool DataController::isClientLoyal(int clientId) {
    // Client becomes loyal starting from the 3rd order.
    // So if the client has >= 2 orders in the database, they are loyal.
    return getClientOrderCount(clientId) >= 2;
}

