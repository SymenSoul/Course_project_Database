#include "mainwindow.h"
#include "../controller/authcontroller.h"
#include "../controller/datacontroller.h"
#include "authwindow.h"
#include "dialogs.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    
    // Refresh all data
    refreshOrders();
    refreshClients();
    refreshServiceTypes();
    refreshBranches();
    
    applyPermissions();
}

void MainWindow::setupUI() {
    resize(1000, 700);
    setWindowTitle("Система управления химчисткой");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // Header panel (User Greeting & Role)
    QHBoxLayout *headerLayout = new QHBoxLayout();
    lblUserGreeting = new QLabel(this);
    User curUser = AuthController::instance().currentUser();
    QString roleRu = (curUser.role == "Admin") ? "Администратор" : "Сотрудник";
    lblUserGreeting->setText(QString("Пользователь: <b>%1</b> | Роль: <i>%2</i>").arg(curUser.fullName, roleRu));
    lblUserGreeting->setStyleSheet("color: #a1a1aa; font-size: 14px;");
    headerLayout->addWidget(lblUserGreeting);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Tab Widget
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    // Connect tab change to refresh appropriate tables
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // =========================================================================
    // Tab 1: Orders (Заказы)
    // =========================================================================
    tabOrders = new QWidget(this);
    QHBoxLayout *ordersLayout = new QHBoxLayout(tabOrders);
    ordersLayout->setContentsMargins(10, 10, 10, 10);
    
    QSplitter *splitter = new QSplitter(Qt::Horizontal, tabOrders);
    ordersLayout->addWidget(splitter);

    // Left side: Orders list
    QWidget *leftOrderWidget = new QWidget(this);
    QVBoxLayout *leftOrderLayout = new QVBoxLayout(leftOrderWidget);
    leftOrderLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *lblOrdersTitle = new QLabel("<b>Оформленные заказы:</b>", this);
    leftOrderLayout->addWidget(lblOrdersTitle);

    viewOrders = new QTableView(this);
    viewOrders->setSelectionBehavior(QAbstractItemView::SelectRows);
    viewOrders->setSelectionMode(QAbstractItemView::SingleSelection);
    viewOrders->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewOrders->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    viewOrders->setAlternatingRowColors(true);
    leftOrderLayout->addWidget(viewOrders);

    QHBoxLayout *orderCrudButtons = new QHBoxLayout();
    btnAddOrder = new QPushButton("Оформить заказ", this);
    btnDeleteOrder = new QPushButton("Удалить заказ", this);
    btnDeleteOrder->setObjectName("btnDelete");
    orderCrudButtons->addWidget(btnAddOrder);
    orderCrudButtons->addWidget(btnDeleteOrder);
    leftOrderLayout->addLayout(orderCrudButtons);
    
    splitter->addWidget(leftOrderWidget);

    // Right side: Order Details & Positions
    boxOrderDetails = new QGroupBox("Состав выбранного заказа", this);
    QVBoxLayout *rightOrderLayout = new QVBoxLayout(boxOrderDetails);
    rightOrderLayout->setContentsMargins(15, 20, 15, 15);
    rightOrderLayout->setSpacing(10);

    lblOrderSummary = new QLabel("Выберите заказ для просмотра деталей.", this);
    lblOrderSummary->setWordWrap(true);
    lblOrderSummary->setStyleSheet("color: #a1a1aa;");
    rightOrderLayout->addWidget(lblOrderSummary);

    viewPositions = new QTableView(this);
    viewPositions->setSelectionBehavior(QAbstractItemView::SelectRows);
    viewPositions->setSelectionMode(QAbstractItemView::SingleSelection);
    viewPositions->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewPositions->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    viewPositions->setAlternatingRowColors(true);
    rightOrderLayout->addWidget(viewPositions);

    QHBoxLayout *posButtonsLayout = new QHBoxLayout();
    btnAddItem = new QPushButton("Добавить вещь", this);
    btnDeleteItem = new QPushButton("Удалить вещь", this);
    btnDeleteItem->setObjectName("btnDelete");
    btnReturnItem = new QPushButton("Выдать вещь (возврат)", this);
    btnReturnItem->setObjectName("btnSecondary");

    posButtonsLayout->addWidget(btnAddItem);
    posButtonsLayout->addWidget(btnDeleteItem);
    posButtonsLayout->addWidget(btnReturnItem);
    rightOrderLayout->addLayout(posButtonsLayout);

    // Disable position actions initially
    btnAddItem->setEnabled(false);
    btnDeleteItem->setEnabled(false);
    btnReturnItem->setEnabled(false);

    splitter->addWidget(boxOrderDetails);
    splitter->setSizes(QList<int>() << 600 << 400);

    tabWidget->addTab(tabOrders, "Заказы");

    // Connect selection model of orders
    connect(viewOrders, &QTableView::clicked, this, [this](const QModelIndex &idx) {
        if (!idx.isValid()) return;
        int row = idx.row();
        int orderId = modelOrders->item(row, 0)->text().toInt();
        m_selectedOrderId = orderId;
        
        QString client = modelOrders->item(row, 1)->text();
        QString date = modelOrders->item(row, 2)->text();
        QString discount = modelOrders->item(row, 5)->text();
        QString total = modelOrders->item(row, 6)->text();
        
        lblOrderSummary->setText(QString("<b>Заказ №%1</b> | Клиент: %2\nПринят: %3 | Скидка: %4 | Сумма: %5 руб.")
                                 .arg(QString::number(orderId), client, date, discount, total));
        
        refreshOrderPositions(orderId);
        btnAddItem->setEnabled(true);
    });

    connect(viewPositions, &QTableView::clicked, this, [this](const QModelIndex &idx) {
        if (!idx.isValid()) return;
        btnDeleteItem->setEnabled(true);
        // Only enable return button if the item is not already returned
        int row = idx.row();
        QString retDate = modelPositions->item(row, 6)->text();
        btnReturnItem->setEnabled(retDate == "В обработке");
    });

    // Connections for Order CRUD
    connect(btnAddOrder, &QPushButton::clicked, this, &MainWindow::handleAddOrder);
    connect(btnDeleteOrder, &QPushButton::clicked, this, &MainWindow::handleDeleteOrder);
    
    // Connections for Positions CRUD
    connect(btnAddItem, &QPushButton::clicked, this, &MainWindow::handleAddItem);
    connect(btnDeleteItem, &QPushButton::clicked, this, &MainWindow::handleDeleteItem);
    connect(btnReturnItem, &QPushButton::clicked, this, &MainWindow::handleMarkReturned);

    // =========================================================================
    // Tab 2: Clients (Клиенты)
    // =========================================================================
    tabClients = new QWidget(this);
    QVBoxLayout *clientsLayout = new QVBoxLayout(tabClients);
    clientsLayout->setContentsMargins(15, 15, 15, 15);
    clientsLayout->setSpacing(10);

    viewClients = new QTableView(this);
    viewClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    viewClients->setSelectionMode(QAbstractItemView::SingleSelection);
    viewClients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewClients->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    viewClients->setAlternatingRowColors(true);
    clientsLayout->addWidget(viewClients);

    QHBoxLayout *clientButtonsLayout = new QHBoxLayout();
    btnAddClient = new QPushButton("Добавить клиента", this);
    btnEditClient = new QPushButton("Редактировать", this);
    btnDeleteClient = new QPushButton("Удалить", this);
    btnDeleteClient->setObjectName("btnDelete");

    clientButtonsLayout->addWidget(btnAddClient);
    clientButtonsLayout->addWidget(btnEditClient);
    clientButtonsLayout->addWidget(btnDeleteClient);
    clientsLayout->addLayout(clientButtonsLayout);

    tabWidget->addTab(tabClients, "Клиенты");

    connect(btnAddClient, &QPushButton::clicked, this, &MainWindow::handleAddClient);
    connect(btnEditClient, &QPushButton::clicked, this, &MainWindow::handleEditClient);
    connect(btnDeleteClient, &QPushButton::clicked, this, &MainWindow::handleDeleteClient);

    // =========================================================================
    // Tab 3: Service Types (Виды услуг)
    // =========================================================================
    tabServices = new QWidget(this);
    QVBoxLayout *servicesLayout = new QVBoxLayout(tabServices);
    servicesLayout->setContentsMargins(15, 15, 15, 15);
    servicesLayout->setSpacing(10);

    viewServices = new QTableView(this);
    viewServices->setSelectionBehavior(QAbstractItemView::SelectRows);
    viewServices->setSelectionMode(QAbstractItemView::SingleSelection);
    viewServices->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewServices->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    viewServices->setAlternatingRowColors(true);
    servicesLayout->addWidget(viewServices);

    QHBoxLayout *serviceButtonsLayout = new QHBoxLayout();
    btnAddService = new QPushButton("Добавить услугу", this);
    btnEditService = new QPushButton("Редактировать", this);
    btnDeleteService = new QPushButton("Удалить", this);
    btnDeleteService->setObjectName("btnDelete");

    serviceButtonsLayout->addWidget(btnAddService);
    serviceButtonsLayout->addWidget(btnEditService);
    serviceButtonsLayout->addWidget(btnDeleteService);
    servicesLayout->addLayout(serviceButtonsLayout);

    tabWidget->addTab(tabServices, "Услуги");

    connect(btnAddService, &QPushButton::clicked, this, &MainWindow::handleAddServiceType);
    connect(btnEditService, &QPushButton::clicked, this, &MainWindow::handleEditServiceType);
    connect(btnDeleteService, &QPushButton::clicked, this, &MainWindow::handleDeleteServiceType);

    // =========================================================================
    // Tab 4: Branches (Филиалы)
    // =========================================================================
    tabBranches = new QWidget(this);
    QVBoxLayout *branchesLayout = new QVBoxLayout(tabBranches);
    branchesLayout->setContentsMargins(15, 15, 15, 15);
    branchesLayout->setSpacing(10);

    viewBranches = new QTableView(this);
    viewBranches->setSelectionBehavior(QAbstractItemView::SelectRows);
    viewBranches->setSelectionMode(QAbstractItemView::SingleSelection);
    viewBranches->setEditTriggers(QAbstractItemView::NoEditTriggers);
    viewBranches->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    viewBranches->setAlternatingRowColors(true);
    branchesLayout->addWidget(viewBranches);

    QHBoxLayout *branchButtonsLayout = new QHBoxLayout();
    btnAddBranch = new QPushButton("Добавить филиал", this);
    btnEditBranch = new QPushButton("Редактировать", this);
    btnDeleteBranch = new QPushButton("Удалить", this);
    btnDeleteBranch->setObjectName("btnDelete");

    branchButtonsLayout->addWidget(btnAddBranch);
    branchButtonsLayout->addWidget(btnEditBranch);
    branchButtonsLayout->addWidget(btnDeleteBranch);
    branchesLayout->addLayout(branchButtonsLayout);

    tabWidget->addTab(tabBranches, "Филиалы");

    connect(btnAddBranch, &QPushButton::clicked, this, &MainWindow::handleAddBranch);
    connect(btnEditBranch, &QPushButton::clicked, this, &MainWindow::handleEditBranch);
    connect(btnDeleteBranch, &QPushButton::clicked, this, &MainWindow::handleDeleteBranch);

    // =========================================================================
    // Tab 5: My Profile (Мой профиль)
    // =========================================================================
    tabProfile = new QWidget(this);
    QVBoxLayout *profileLayout = new QVBoxLayout(tabProfile);
    profileLayout->setContentsMargins(50, 40, 50, 40);
    profileLayout->setSpacing(20);

    QGroupBox *profileGroup = new QGroupBox("Ваши регистрационные данные", this);
    QFormLayout *profileForm = new QFormLayout(profileGroup);
    profileForm->setContentsMargins(20, 25, 20, 20);
    profileForm->setSpacing(15);

    txtProfileUsername = new QLineEdit(this);
    profileForm->addRow("Имя пользователя (логин):", txtProfileUsername);

    txtProfileFullName = new QLineEdit(this);
    profileForm->addRow("ФИО сотрудника:", txtProfileFullName);

    txtProfilePassword = new QLineEdit(this);
    txtProfilePassword->setEchoMode(QLineEdit::Password);
    txtProfilePassword->setPlaceholderText("Оставьте пустым, если не хотите менять");
    profileForm->addRow("Новый пароль:", txtProfilePassword);

    txtProfileConfirmPassword = new QLineEdit(this);
    txtProfileConfirmPassword->setEchoMode(QLineEdit::Password);
    profileForm->addRow("Подтверждение пароля:", txtProfileConfirmPassword);

    profileLayout->addWidget(profileGroup);

    QHBoxLayout *profileButtons = new QHBoxLayout();
    btnSaveProfile = new QPushButton("Сохранить изменения", this);
    btnLogout = new QPushButton("Выйти из системы", this);
    btnLogout->setObjectName("btnCancel");
    
    profileButtons->addWidget(btnSaveProfile);
    profileButtons->addStretch();
    profileButtons->addWidget(btnLogout);
    profileLayout->addLayout(profileButtons);
    profileLayout->addStretch();

    tabWidget->addTab(tabProfile, "Мой профиль");

    // Set initial values
    txtProfileUsername->setText(curUser.username);
    txtProfileFullName->setText(curUser.fullName);

    connect(btnSaveProfile, &QPushButton::clicked, this, &MainWindow::handleUpdateProfile);
    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::handleLogout);

    // Initialize models
    modelOrders = new QStandardItemModel(this);
    modelClients = new QStandardItemModel(this);
    modelServices = new QStandardItemModel(this);
    modelBranches = new QStandardItemModel(this);
    modelPositions = new QStandardItemModel(this);

    viewOrders->setModel(modelOrders);
    viewClients->setModel(modelClients);
    viewServices->setModel(modelServices);
    viewBranches->setModel(modelBranches);
    viewPositions->setModel(modelPositions);
}

void MainWindow::applyPermissions() {
    User user = AuthController::instance().currentUser();
    if (user.role == "Employee") {
        // Employees cannot delete orders or clients
        btnDeleteOrder->hide();
        btnDeleteClient->hide();

        // Employees cannot modify dictionary tables (Services and Branches)
        btnAddService->hide();
        btnEditService->hide();
        btnDeleteService->hide();

        btnAddBranch->hide();
        btnEditBranch->hide();
        btnDeleteBranch->hide();
    }
}

void MainWindow::onTabChanged(int index) {
    switch (index) {
        case 0: refreshOrders(); break;
        case 1: refreshClients(); break;
        case 2: refreshServiceTypes(); break;
        case 3: refreshBranches(); break;
        default: break;
    }
}

// =========================================================================
// Data Refresh Functions
// =========================================================================

void MainWindow::refreshOrders() {}
void MainWindow::refreshClients() {}
void MainWindow::refreshServiceTypes() {}
void MainWindow::refreshBranches() {}
void MainWindow::refreshOrderPositions(int) {}