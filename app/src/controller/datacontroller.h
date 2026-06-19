#ifndef DATACONTROLLER_H
#define DATACONTROLLER_H

#include <QObject>
#include <QList>
#include "../model/entities.h"

class DataController : public QObject {
    Q_OBJECT
public:
    explicit DataController(QObject *parent = nullptr);
    static DataController& instance();

    // Branch CRUD
    QList<Branch> getAllBranches();
    bool addBranch(const QString& name, const QString& address);
    bool updateBranch(int id, const QString& name, const QString& address);
    bool deleteBranch(int id, QString& errorMsg);

    // ServiceType CRUD
    QList<ServiceType> getAllServiceTypes();
    bool addServiceType(const QString& name, const QString& type, double baseCost);
    bool updateServiceType(int id, const QString& name, const QString& type, double baseCost);
    bool deleteServiceType(int id, QString& errorMsg);

    // Client CRUD
    QList<Client> getAllClients();
    bool addClient(const QString& lastName, const QString& firstName, const QString& middleName, const QString& phone);
    bool updateClient(int id, const QString& lastName, const QString& firstName, const QString& middleName, const QString& phone);
    bool deleteClient(int id, QString& errorMsg);
    int getClientOrderCount(int clientId);
    bool isClientLoyal(int clientId);

    // Order CRUD
    QList<Order> getAllOrders();
    QList<Order> getOrdersByClient(int clientId);
    bool addOrder(int branchId, int userId, int clientId, QString& errorMsg);
    bool deleteOrder(int id, QString& errorMsg);

    // OrderPosition CRUD
    QList<OrderPosition> getPositionsForOrder(int orderId);
    bool addOrderPosition(int orderId, int serviceTypeId, const QString& itemName, double complexity, double urgency, double workVolume, QString& errorMsg);
    bool deleteOrderPosition(int id, QString& errorMsg);
    bool markPositionAsReturned(int id, QString& errorMsg);
};

#endif // DATACONTROLLER_H
