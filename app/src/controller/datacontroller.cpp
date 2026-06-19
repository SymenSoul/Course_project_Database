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

// =========================================================================
// Order CRUD
// =========================================================================

QList<Order> DataController::getAllOrders() {
    QList<Order> list;
    if (!Database::instance().isOpen()) return list;

    // Use complex join query to retrieve all display fields and dynamically calculated total price
    QString queryStr = 
        "SELECT o.*, "
        "       b.name AS branch_name, "
        "       u.full_name AS employee_name, "
        "       c.last_name || ' ' || c.first_name || ' ' || COALESCE(c.middle_name, '') AS client_name, "
        "       COALESCE((SELECT SUM(final_price) FROM order_position_view WHERE order_id = o.id), 0.00) AS total_price "
        "FROM orders o "
        "JOIN branch b ON o.branch_id = b.id "
        "JOIN users u ON o.user_id = u.id "
        "JOIN client c ON o.client_id = c.id "
        "ORDER BY o.intake_date DESC";

    QSqlQuery q(queryStr, Database::instance().db());
    while (q.next()) {
        list.append(Order::fromQuery(q));
    }
    return list;
}

QList<Order> DataController::getOrdersByClient(int clientId) {
    QList<Order> list;
    if (!Database::instance().isOpen()) return list;

    QString queryStr = 
        "SELECT o.*, "
        "       b.name AS branch_name, "
        "       u.full_name AS employee_name, "
        "       c.last_name || ' ' || c.first_name || ' ' || COALESCE(c.middle_name, '') AS client_name, "
        "       COALESCE((SELECT SUM(final_price) FROM order_position_view WHERE order_id = o.id), 0.00) AS total_price "
        "FROM orders o "
        "JOIN branch b ON o.branch_id = b.id "
        "JOIN users u ON o.user_id = u.id "
        "JOIN client c ON o.client_id = c.id "
        "WHERE o.client_id = :client_id "
        "ORDER BY o.intake_date DESC";

    QSqlQuery q(Database::instance().db());
    q.prepare(queryStr);
    q.bindValue(":client_id", clientId);
    if (q.exec()) {
        while (q.next()) {
            list.append(Order::fromQuery(q));
        }
    }
    return list;
}

bool DataController::addOrder(int branchId, int userId, int clientId, QString& errorMsg) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    // Note: discount is set automatically by the database trigger (trigger_set_order_discount)
    q.prepare("INSERT INTO orders (branch_id, user_id, client_id, intake_date) "
              "VALUES (:branch_id, :user_id, :client_id, CURRENT_TIMESTAMP)");
    q.bindValue(":branch_id", branchId);
    q.bindValue(":user_id", userId);
    q.bindValue(":client_id", clientId);

    if (!q.exec()) {
        errorMsg = "Ошибка создания заказа: " + q.lastError().text();
        return false;
    }
    return true;
}

bool DataController::deleteOrder(int id, QString& errorMsg) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("DELETE FROM orders WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        errorMsg = "Ошибка при удалении заказа: " + q.lastError().text();
        return false;
    }
    return true;
}

// =========================================================================
// OrderPosition CRUD
// =========================================================================

QList<OrderPosition> DataController::getPositionsForOrder(int orderId) {
    QList<OrderPosition> list;
    if (!Database::instance().isOpen()) return list;

    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT * "
              "FROM order_position_view "
              "WHERE order_id = :order_id "
              "ORDER BY id ASC");
    q.bindValue(":order_id", orderId);

    if (q.exec()) {
        while (q.next()) {
            list.append(OrderPosition::fromQuery(q));
        }
    }
    return list;
}

bool DataController::addOrderPosition(int orderId, int serviceTypeId, const QString& itemName, double complexity, double urgency, double workVolume, QString& errorMsg) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    // Note: final_price is calculated dynamically in the database view (order_position_view)
    q.prepare("INSERT INTO order_position (order_id, service_type_id, item_name, complexity, urgency, work_volume) "
              "VALUES (:order_id, :service_type_id, :item_name, :complexity, :urgency, :work_volume)");
    q.bindValue(":order_id", orderId);
    q.bindValue(":service_type_id", serviceTypeId);
    q.bindValue(":item_name", itemName);
    q.bindValue(":complexity", complexity);
    q.bindValue(":urgency", urgency);
    q.bindValue(":work_volume", workVolume);

    if (!q.exec()) {
        errorMsg = "Ошибка добавления вещи в заказ: " + q.lastError().text();
        return false;
    }
    return true;
}

bool DataController::deleteOrderPosition(int id, QString& errorMsg) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("DELETE FROM order_position WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        errorMsg = "Ошибка удаления вещи: " + q.lastError().text();
        return false;
    }
    return true;
}

bool DataController::markPositionAsReturned(int id, QString& errorMsg) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("UPDATE order_position SET return_date = CURRENT_TIMESTAMP WHERE id = :id");
    q.bindValue(":id", id);
    if (!q.exec()) {
        errorMsg = "Ошибка выдачи вещи: " + q.lastError().text();
        return false;
    }
    return true;
}
