#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include "../model/entities.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    // Auth & Profile
    void handleLogout();
    void handleUpdateProfile();

    // Navigation & Table selection
    void onTabChanged(int index);

    // Client CRUD slots
    void handleAddClient();
    void handleEditClient();
    void handleDeleteClient();

    // Order CRUD slots
    void handleAddOrder();
    void handleDeleteOrder();

    // OrderPosition (Items) slots
    void handleAddItem();
    void handleDeleteItem();
    void handleMarkReturned();

    // Branch CRUD slots
    void handleAddBranch();
    void handleEditBranch();
    void handleDeleteBranch();

    // ServiceType CRUD slots
    void handleAddServiceType();
    void handleEditServiceType();
    void handleDeleteServiceType();

private:
    void setupUI();
    void applyPermissions();
    
    // Refresh table methods
    void refreshOrders();
    void refreshClients();
    void refreshServiceTypes();
    void refreshBranches();
    void refreshOrderPositions(int orderId);

    // Tab Widget and tabs
    QTabWidget *tabWidget;
    QWidget *tabOrders;
    QWidget *tabClients;
    QWidget *tabServices;
    QWidget *tabBranches;
    QWidget *tabProfile;

    // Greeting info
    QLabel *lblUserGreeting;

    // Models
    QStandardItemModel *modelOrders;
    QStandardItemModel *modelClients;
    QStandardItemModel *modelServices;
    QStandardItemModel *modelBranches;
    QStandardItemModel *modelPositions;

    // Table Views
    QTableView *viewOrders;
    QTableView *viewClients;
    QTableView *viewServices;
    QTableView *viewBranches;
    QTableView *viewPositions;

    // Order detail view elements
    QGroupBox *boxOrderDetails;
    QLabel *lblOrderSummary;
    QPushButton *btnAddItem;
    QPushButton *btnDeleteItem;
    QPushButton *btnReturnItem;
    int m_selectedOrderId = -1;

    // Client tab buttons
    QPushButton *btnAddClient;
    QPushButton *btnEditClient;
    QPushButton *btnDeleteClient;

    // Order tab buttons
    QPushButton *btnAddOrder;
    QPushButton *btnDeleteOrder;

    // Branch tab buttons
    QPushButton *btnAddBranch;
    QPushButton *btnEditBranch;
    QPushButton *btnDeleteBranch;

    // ServiceType tab buttons
    QPushButton *btnAddService;
    QPushButton *btnEditService;
    QPushButton *btnDeleteService;

    // Profile tab inputs
    QLineEdit *txtProfileUsername;
    QLineEdit *txtProfileFullName;
    QLineEdit *txtProfilePassword;
    QLineEdit *txtProfileConfirmPassword;
    QPushButton *btnSaveProfile;
    QPushButton *btnLogout;
};

#endif // MAINWINDOW_H
