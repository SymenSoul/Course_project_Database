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

void MainWindow::refreshOrders() {
    modelOrders->clear();
    modelOrders->setHorizontalHeaderLabels(QStringList() << "ID" << "Клиент" << "Дата приема" << "Филиал" << "Сотрудник" << "Скидка" << "Итоговая сумма");

    QList<Order> list = DataController::instance().getAllOrders();
    for (const auto& o : list) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(QString::number(o.id)));
        row.append(new QStandardItem(o.clientName));
        row.append(new QStandardItem(o.intakeDate.toString("dd.MM.yyyy hh:mm")));
        row.append(new QStandardItem(o.branchName));
        row.append(new QStandardItem(o.employeeName));
        row.append(new QStandardItem(QString::number(o.discount * 100) + "%"));
        row.append(new QStandardItem(QString::number(o.totalPrice, 'f', 2)));
        modelOrders->appendRow(row);
    }
    viewOrders->resizeColumnsToContents();
    
    // Clear details if previously selected order doesn't exist
    m_selectedOrderId = -1;
    lblOrderSummary->setText("Выберите заказ для просмотра деталей.");
    modelPositions->clear();
    btnAddItem->setEnabled(false);
    btnDeleteItem->setEnabled(false);
    btnReturnItem->setEnabled(false);
}

void MainWindow::refreshClients() {
    modelClients->clear();
    modelClients->setHorizontalHeaderLabels(QStringList() << "ID" << "Фамилия" << "Имя" << "Отчество" << "Телефон" << "Статус клиента" << "Кол-во заказов");

    QList<Client> list = DataController::instance().getAllClients();
    for (const auto& c : list) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(QString::number(c.id)));
        row.append(new QStandardItem(c.lastName));
        row.append(new QStandardItem(c.firstName));
        row.append(new QStandardItem(c.middleName));
        row.append(new QStandardItem(c.phone));
        
        int orderCount = DataController::instance().getClientOrderCount(c.id);
        QString status = (orderCount >= 2) ? "Постоянный (скидка 3%)" : "Обычный";
        row.append(new QStandardItem(status));
        row.append(new QStandardItem(QString::number(orderCount)));
        
        modelClients->appendRow(row);
    }
    viewClients->resizeColumnsToContents();
}

void MainWindow::refreshServiceTypes() {
    modelServices->clear();
    modelServices->setHorizontalHeaderLabels(QStringList() << "ID" << "Название услуги" << "Категория" << "Базовая стоимость");

    QList<ServiceType> list = DataController::instance().getAllServiceTypes();
    for (const auto& s : list) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(QString::number(s.id)));
        row.append(new QStandardItem(s.name));
        row.append(new QStandardItem(s.type));
        row.append(new QStandardItem(QString::number(s.baseCost, 'f', 2) + " руб."));
        modelServices->appendRow(row);
    }
    viewServices->resizeColumnsToContents();
}

void MainWindow::refreshBranches() {
    modelBranches->clear();
    modelBranches->setHorizontalHeaderLabels(QStringList() << "ID" << "Название филиала" << "Адрес");

    QList<Branch> list = DataController::instance().getAllBranches();
    for (const auto& b : list) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(QString::number(b.id)));
        row.append(new QStandardItem(b.name));
        row.append(new QStandardItem(b.address));
        modelBranches->appendRow(row);
    }
    viewBranches->resizeColumnsToContents();
}

void MainWindow::refreshOrderPositions(int orderId) {
    modelPositions->clear();
    modelPositions->setHorizontalHeaderLabels(QStringList() << "ID" << "Вещь" << "Услуга" << "Объем работ" << "Сложность" << "Срочность" << "Дата возврата" << "Стоимость");

    QList<OrderPosition> list = DataController::instance().getPositionsForOrder(orderId);
    for (const auto& op : list) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(QString::number(op.id)));
        row.append(new QStandardItem(op.itemName));
        row.append(new QStandardItem(op.serviceName));
        row.append(new QStandardItem(QString::number(op.workVolume)));
        row.append(new QStandardItem(QString::number(op.complexity)));
        row.append(new QStandardItem(QString::number(op.urgency)));
        
        QString dateStr = op.isReturned ? op.returnDate.toString("dd.MM.yyyy hh:mm") : "В обработке";
        row.append(new QStandardItem(dateStr));
        row.append(new QStandardItem(QString::number(op.finalPrice, 'f', 2) + " руб."));
        modelPositions->appendRow(row);
    }
    viewPositions->resizeColumnsToContents();
    
    // Clear selection state
    btnDeleteItem->setEnabled(false);
    btnReturnItem->setEnabled(false);
}

// =========================================================================
// Client CRUD Implementation
// =========================================================================

void MainWindow::handleAddClient() {
    ClientDialog dlg(this, "Добавление клиента");
    if (dlg.exec() == QDialog::Accepted) {
        QString lastName, firstName, middleName, phone;
        dlg.getClientData(lastName, firstName, middleName, phone);
        if (DataController::instance().addClient(lastName, firstName, middleName, phone)) {
            refreshClients();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить клиента.");
        }
    }
}

void MainWindow::handleEditClient() {
    QModelIndexList selected = viewClients->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите клиента для редактирования.");
        return;
    }
    int row = selected.first().row();
    int id = modelClients->item(row, 0)->text().toInt();
    QString lastName = modelClients->item(row, 1)->text();
    QString firstName = modelClients->item(row, 2)->text();
    QString middleName = modelClients->item(row, 3)->text();
    QString phone = modelClients->item(row, 4)->text();

    ClientDialog dlg(this, "Редактирование клиента");
    dlg.setClientData(lastName, firstName, middleName, phone);
    if (dlg.exec() == QDialog::Accepted) {
        QString newLastName, newFirstName, newMiddleName, newPhone;
        dlg.getClientData(newLastName, newFirstName, newMiddleName, newPhone);
        if (DataController::instance().updateClient(id, newLastName, newFirstName, newMiddleName, newPhone)) {
            refreshClients();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить данные клиента.");
        }
    }
}

void MainWindow::handleDeleteClient() {
    QModelIndexList selected = viewClients->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите клиента для удаления.");
        return;
    }
    int row = selected.first().row();
    int id = modelClients->item(row, 0)->text().toInt();
    QString clientName = modelClients->item(row, 1)->text() + " " + modelClients->item(row, 2)->text();

    if (QMessageBox::question(this, "Подтверждение", QString("Вы уверены, что хотите удалить клиента %1?").arg(clientName)) == QMessageBox::Yes) {
        QString errorMsg;
        if (DataController::instance().deleteClient(id, errorMsg)) {
            refreshClients();
        } else {
            QMessageBox::critical(this, "Ошибка", errorMsg);
        }
    }
}

// =========================================================================
// Order & Positions CRUD Implementation
// =========================================================================

void MainWindow::handleAddOrder() {
    OrderDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        int branchId = dlg.getSelectedBranchId();
        int clientId = dlg.getSelectedClientId();
        int userId = AuthController::instance().currentUser().id;

        QString errorMsg;
        if (DataController::instance().addOrder(branchId, userId, clientId, errorMsg)) {
            refreshOrders();
        } else {
            QMessageBox::critical(this, "Ошибка", errorMsg);
        }
    }
}

void MainWindow::handleDeleteOrder() {
    QModelIndexList selected = viewOrders->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите заказ для удаления.");
        return;
    }
    int row = selected.first().row();
    int id = modelOrders->item(row, 0)->text().toInt();

    if (QMessageBox::question(this, "Подтверждение", QString("Вы уверены, что хотите удалить заказ №%1 со всеми его позициями?").arg(id)) == QMessageBox::Yes) {
        QString errorMsg;
        if (DataController::instance().deleteOrder(id, errorMsg)) {
            refreshOrders();
        } else {
            QMessageBox::critical(this, "Ошибка", errorMsg);
        }
    }
}

void MainWindow::handleAddItem() {
    if (m_selectedOrderId == -1) return;

    OrderPositionDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        int serviceId = dlg.getSelectedServiceId();
        QString itemName = dlg.getItemName();
        double complexity = dlg.getComplexity();
        double urgency = dlg.getUrgency();
        double workVolume = dlg.getWorkVolume();

        QString errorMsg;
        if (DataController::instance().addOrderPosition(m_selectedOrderId, serviceId, itemName, complexity, urgency, workVolume, errorMsg)) {
            refreshOrderPositions(m_selectedOrderId);
            // Also refresh orders table to update total price
            int currentTab = tabWidget->currentIndex();
            refreshOrders();
            tabWidget->setCurrentIndex(currentTab);
        } else {
            QMessageBox::critical(this, "Ошибка", errorMsg);
        }
    }
}

void MainWindow::handleDeleteItem() {
    QModelIndexList selected = viewPositions->selectionModel()->selectedRows();
    if (selected.isEmpty() || m_selectedOrderId == -1) return;

    int row = selected.first().row();
    int id = modelPositions->item(row, 0)->text().toInt();
    QString itemName = modelPositions->item(row, 1)->text();

    if (QMessageBox::question(this, "Подтверждение", QString("Вы уверены, что хотите удалить вещь '%1' из заказа?").arg(itemName)) == QMessageBox::Yes) {
        QString errorMsg;
        if (DataController::instance().deleteOrderPosition(id, errorMsg)) {
            refreshOrderPositions(m_selectedOrderId);
            // Refresh orders table to update total price
            refreshOrders();
        } else {
            QMessageBox::critical(this, "Ошибка", errorMsg);
        }
    }
}

void MainWindow::handleMarkReturned() {
    QModelIndexList selected = viewPositions->selectionModel()->selectedRows();
    if (selected.isEmpty() || m_selectedOrderId == -1) return;

    int row = selected.first().row();
    int id = modelPositions->item(row, 0)->text().toInt();
    QString itemName = modelPositions->item(row, 1)->text();

    if (QMessageBox::question(this, "Подтверждение", QString("Отметить выдачу (возврат клиенту) вещи '%1'?").arg(itemName)) == QMessageBox::Yes) {
        QString errorMsg;
        if (DataController::instance().markPositionAsReturned(id, errorMsg)) {
            refreshOrderPositions(m_selectedOrderId);
        } else {
            QMessageBox::critical(this, "Ошибка", errorMsg);
        }
    }
}

// =========================================================================
// Branch CRUD Implementation
// =========================================================================

void MainWindow::handleAddBranch() {
    BranchDialog dlg(this, "Добавление филиала");
    if (dlg.exec() == QDialog::Accepted) {
        QString name, address;
        dlg.getBranchData(name, address);
        if (DataController::instance().addBranch(name, address)) {
            refreshBranches();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать филиал.");
        }
    }
}

void MainWindow::handleEditBranch() {
    QModelIndexList selected = viewBranches->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите филиал для редактирования.");
        return;
    }
    int row = selected.first().row();
    int id = modelBranches->item(row, 0)->text().toInt();
    QString name = modelBranches->item(row, 1)->text();
    QString address = modelBranches->item(row, 2)->text();

    BranchDialog dlg(this, "Редактирование филиала");
    dlg.setBranchData(name, address);
    if (dlg.exec() == QDialog::Accepted) {
        QString newName, newAddress;
        dlg.getBranchData(newName, newAddress);
        if (DataController::instance().updateBranch(id, newName, newAddress)) {
            refreshBranches();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить филиал.");
        }
    }
}

void MainWindow::handleDeleteBranch() {
    QModelIndexList selected = viewBranches->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите филиал для удаления.");
        return;
    }
    int row = selected.first().row();
    int id = modelBranches->item(row, 0)->text().toInt();
    QString name = modelBranches->item(row, 1)->text();

    if (QMessageBox::question(this, "Подтверждение", QString("Удалить филиал '%1'?").arg(name)) == QMessageBox::Yes) {
        QString errorMsg;
        if (DataController::instance().deleteBranch(id, errorMsg)) {
            refreshBranches();
        } else {
            QMessageBox::critical(this, "Ошибка", errorMsg);
        }
    }
}

// =========================================================================
// ServiceType CRUD Implementation
// =========================================================================

void MainWindow::handleAddServiceType() {
    ServiceTypeDialog dlg(this, "Добавление услуги");
    if (dlg.exec() == QDialog::Accepted) {
        QString name, type;
        double baseCost;
        dlg.getServiceData(name, type, baseCost);
        if (DataController::instance().addServiceType(name, type, baseCost)) {
            refreshServiceTypes();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать услугу.");
        }
    }
}

void MainWindow::handleEditServiceType() {
    QModelIndexList selected = viewServices->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите услугу для редактирования.");
        return;
    }
    int row = selected.first().row();
    int id = modelServices->item(row, 0)->text().toInt();
    QString name = modelServices->item(row, 1)->text();
    QString type = modelServices->item(row, 2)->text();
    // Parse double cost, removing " руб." suffix
    QString costStr = modelServices->item(row, 3)->text();
    costStr.remove(" руб.");
    double baseCost = costStr.toDouble();

    ServiceTypeDialog dlg(this, "Редактирование услуги");
    dlg.setServiceData(name, type, baseCost);
    if (dlg.exec() == QDialog::Accepted) {
        QString newName, newType;
        double newBaseCost;
        dlg.getServiceData(newName, newType, newBaseCost);
        if (DataController::instance().updateServiceType(id, newName, newType, newBaseCost)) {
            refreshServiceTypes();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить услугу.");
        }
    }
}

void MainWindow::handleDeleteServiceType() {
    QModelIndexList selected = viewServices->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите услугу для удаления.");
        return;
    }
    int row = selected.first().row();
    int id = modelServices->item(row, 0)->text().toInt();
    QString name = modelServices->item(row, 1)->text();

    if (QMessageBox::question(this, "Подтверждение", QString("Удалить услугу '%1'?").arg(name)) == QMessageBox::Yes) {
        QString errorMsg;
        if (DataController::instance().deleteServiceType(id, errorMsg)) {
            refreshServiceTypes();
        } else {
            QMessageBox::critical(this, "Ошибка", errorMsg);
        }
    }
}

// =========================================================================
// Profile & Session management
// =========================================================================

void MainWindow::handleUpdateProfile() {
    int userId = AuthController::instance().currentUser().id;
    QString username = txtProfileUsername->text().trimmed();
    QString fullName = txtProfileFullName->text().trimmed();
    QString password = txtProfilePassword->text();
    QString confirmPassword = txtProfileConfirmPassword->text();
    QString errorMsg;

    if (!password.isEmpty() && password != confirmPassword) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают.");
        return;
    }

    if (AuthController::instance().updateUserProfile(userId, username, password, fullName, errorMsg)) {
        QMessageBox::information(this, "Успех", "Данные успешно обновлены!");
        txtProfilePassword->clear();
        txtProfileConfirmPassword->clear();
        
        // Refresh greeting label
        User curUser = AuthController::instance().currentUser();
        QString roleRu = (curUser.role == "Admin") ? "Администратор" : "Сотрудник";
        lblUserGreeting->setText(QString("Пользователь: <b>%1</b> | Роль: <i>%2</i>").arg(curUser.fullName, roleRu));
    } else {
        QMessageBox::critical(this, "Ошибка", errorMsg);
    }
}

void MainWindow::handleLogout() {
    AuthController::instance().logout();
    AuthWindow *authWin = new AuthWindow();
    authWin->show();
    this->close();
}
