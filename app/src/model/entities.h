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

class Client {
public:
    int id = 0;
    QString lastName;
    QString firstName;
    QString middleName;
    QString phone;

    QString getFullName() const {
        return lastName + " " + firstName + (middleName.isEmpty() ? "" : " " + middleName);
    }

    static Client fromQuery(const QSqlQuery& q) {
        Client c;
        c.id = q.value("id").toInt();
        c.lastName = q.value("last_name").toString();
        c.firstName = q.value("first_name").toString();
        c.middleName = q.value("middle_name").toString();
        c.phone = q.value("phone").toString();
        return c;
    }
};

class Branch {
public:
    int id = 0;
    QString name;
    QString address;

    static Branch fromQuery(const QSqlQuery& q) {
        Branch b;
        b.id = q.value("id").toInt();
        b.name = q.value("name").toString();
        b.address = q.value("address").toString();
        return b;
    }
};

class ServiceType {
public:
    int id = 0;
    QString name;
    QString type;
    double baseCost = 0.0;

    static ServiceType fromQuery(const QSqlQuery& q) {
        ServiceType s;
        s.id = q.value("id").toInt();
        s.name = q.value("name").toString();
        s.type = q.value("type").toString();
        s.baseCost = q.value("base_cost").toDouble();
        return s;
    }
};

class Order {
public:
    int id = 0;
    int branchId = 0;
    int userId = 0;
    int clientId = 0;
    QDateTime intakeDate;
    double discount = 0.0;

    // Join/helper fields for UI display
    QString branchName;
    QString employeeName;
    QString clientName;
    double totalPrice = 0.0;

    static Order fromQuery(const QSqlQuery& q) {
        Order o;
        o.id = q.value("id").toInt();
        o.branchId = q.value("branch_id").toInt();
        o.userId = q.value("user_id").toInt();
        o.clientId = q.value("client_id").toInt();
        o.intakeDate = q.value("intake_date").toDateTime();
        o.discount = q.value("discount").toDouble();
        
        QSqlRecord rec = q.record();
        if (rec.indexOf("branch_name") != -1) {
            o.branchName = q.value("branch_name").toString();
        }
        if (rec.indexOf("employee_name") != -1) {
            o.employeeName = q.value("employee_name").toString();
        }
        if (rec.indexOf("client_name") != -1) {
            o.clientName = q.value("client_name").toString();
        }
        if (rec.indexOf("total_price") != -1) {
            o.totalPrice = q.value("total_price").toDouble();
        }
        return o;
    }
};

class OrderPosition {
public:
    int id = 0;
    int orderId = 0;
    int serviceTypeId = 0;
    QString itemName;
    double complexity = 1.0;
    double urgency = 1.0;
    double workVolume = 1.0;
    double finalPrice = 0.0;
    QDateTime returnDate;
    bool isReturned = false;

    // Join/helper fields for UI display
    QString serviceName;

    static OrderPosition fromQuery(const QSqlQuery& q) {
        OrderPosition op;
        op.id = q.value("id").toInt();
        op.orderId = q.value("order_id").toInt();
        op.serviceTypeId = q.value("service_type_id").toInt();
        op.itemName = q.value("item_name").toString();
        op.complexity = q.value("complexity").toDouble();
        op.urgency = q.value("urgency").toDouble();
        op.workVolume = q.value("work_volume").toDouble();
        op.finalPrice = q.value("final_price").toDouble();
        
        QVariant ret = q.value("return_date");
        if (ret.isValid() && !ret.isNull()) {
            op.returnDate = ret.toDateTime();
            op.isReturned = true;
        } else {
            op.isReturned = false;
        }

        QSqlRecord rec = q.record();
        if (rec.indexOf("service_name") != -1) {
            op.serviceName = q.value("service_name").toString();
        }
        return op;
    }
};

#endif // ENTITIES_H
