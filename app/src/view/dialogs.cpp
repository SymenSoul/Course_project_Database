#include "dialogs.h"
#include "../controller/datacontroller.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

// =========================================================================
// Client Dialog Implementation
// =========================================================================
ClientDialog::ClientDialog(QWidget *parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    txtLastName = new QLineEdit(this);
    txtLastName->setPlaceholderText("Фамилия");
    formLayout->addRow("Фамилия *:", txtLastName);

    txtFirstName = new QLineEdit(this);
    txtFirstName->setPlaceholderText("Имя");
    formLayout->addRow("Имя *:", txtFirstName);

    txtMiddleName = new QLineEdit(this);
    txtMiddleName->setPlaceholderText("Отчество (при наличии)");
    formLayout->addRow("Отчество:", txtMiddleName);

    txtPhone = new QLineEdit(this);
    txtPhone->setPlaceholderText("+7 (XXX) XXX-XX-XX");
    formLayout->addRow("Телефон *:", txtPhone);

    mainLayout->addLayout(formLayout);

    // Dialog buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Сохранить", this);
    QPushButton *btnCancel = new QPushButton("Отмена", this);
    btnCancel->setObjectName("btnCancel");
    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, this, [this]() {
        if (txtLastName->text().trimmed().isEmpty() || txtFirstName->text().trimmed().isEmpty() || txtPhone->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Предупреждение", "Пожалуйста, заполните все поля, отмеченные звездочкой (*).");
            return;
        }
        accept();
    });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void ClientDialog::setClientData(const QString& lastName, const QString& firstName, const QString& middleName, const QString& phone) {
    txtLastName->setText(lastName);
    txtFirstName->setText(firstName);
    txtMiddleName->setText(middleName);
    txtPhone->setText(phone);
}

void ClientDialog::getClientData(QString& lastName, QString& firstName, QString& middleName, QString& phone) const {
    lastName = txtLastName->text().trimmed();
    firstName = txtFirstName->text().trimmed();
    middleName = txtMiddleName->text().trimmed();
    phone = txtPhone->text().trimmed();
}

// =========================================================================
// Order Dialog Implementation
// =========================================================================
OrderDialog::OrderDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Оформление нового заказа");
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);

    cbBranch = new QComboBox(this);
    for (const auto& b : DataController::instance().getAllBranches()) {
        cbBranch->addItem(b.name, b.id);
    }
    formLayout->addRow("Филиал приема:", cbBranch);

    cbClient = new QComboBox(this);
    for (const auto& c : DataController::instance().getAllClients()) {
        cbClient->addItem(c.getFullName() + " (" + c.phone + ")", c.id);
    }
    formLayout->addRow("Клиент:", cbClient);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnCreate = new QPushButton("Создать заказ", this);
    QPushButton *btnCancel = new QPushButton("Отмена", this);
    btnCancel->setObjectName("btnCancel");
    btnLayout->addStretch();
    btnLayout->addWidget(btnCreate);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(btnCreate, &QPushButton::clicked, this, [this]() {
        if (cbBranch->currentIndex() == -1 || cbClient->currentIndex() == -1) {
            QMessageBox::warning(this, "Предупреждение", "Необходимо выбрать филиал и клиента.");
            return;
        }
        accept();
    });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

int OrderDialog::getSelectedBranchId() const {
    return cbBranch->currentData().toInt();
}

int OrderDialog::getSelectedClientId() const {
    return cbClient->currentData().toInt();
}

// =========================================================================
// OrderPosition Dialog Implementation
// =========================================================================
OrderPositionDialog::OrderPositionDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Добавление вещи в заказ");
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    cbService = new QComboBox(this);
    for (const auto& s : DataController::instance().getAllServiceTypes()) {
        cbService->addItem(s.name + " (" + QString::number(s.baseCost, 'f', 2) + " руб.)", s.id);
    }
    formLayout->addRow("Услуга *:", cbService);

    txtItemName = new QLineEdit(this);
    txtItemName->setPlaceholderText("Например: Куртка мужская кожаная");
    formLayout->addRow("Наименование вещи *:", txtItemName);

    sbVolume = new QDoubleSpinBox(this);
    sbVolume->setRange(0.01, 100.0);
    sbVolume->setValue(1.0);
    sbVolume->setSuffix(" шт/кг/м²");
    formLayout->addRow("Объем работ *:", sbVolume);

    cbComplexity = new QComboBox(this);
    cbComplexity->addItem("Стандартная (x1.0)", 1.0);
    cbComplexity->addItem("Средняя (пятна средней сложности x1.3)", 1.3);
    cbComplexity->addItem("Высокая (сложные пятна, деликатная ткань x1.6)", 1.6);
    formLayout->addRow("Сложность работы:", cbComplexity);

    cbUrgency = new QComboBox(this);
    cbUrgency->addItem("Обычная (x1.0)", 1.0);
    cbUrgency->addItem("Срочная (выполнение за сутки x1.3)", 1.3);
    cbUrgency->addItem("Экспресс (выполнение за 3 часа x1.5)", 1.5);
    formLayout->addRow("Срочность работы:", cbUrgency);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("Добавить", this);
    QPushButton *btnCancel = new QPushButton("Отмена", this);
    btnCancel->setObjectName("btnCancel");
    btnLayout->addStretch();
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(btnAdd, &QPushButton::clicked, this, [this]() {
        if (cbService->currentIndex() == -1 || txtItemName->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите услугу и укажите название вещи.");
            return;
        }
        accept();
    });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

int OrderPositionDialog::getSelectedServiceId() const {
    return cbService->currentData().toInt();
}

QString OrderPositionDialog::getItemName() const {
    return txtItemName->text().trimmed();
}

double OrderPositionDialog::getComplexity() const {
    return cbComplexity->currentData().toDouble();
}

double OrderPositionDialog::getUrgency() const {
    return cbUrgency->currentData().toDouble();
}

double OrderPositionDialog::getWorkVolume() const {
    return sbVolume->value();
}

// =========================================================================
// Branch Dialog Implementation
// =========================================================================
BranchDialog::BranchDialog(QWidget *parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    txtName = new QLineEdit(this);
    txtName->setPlaceholderText("Название филиала");
    formLayout->addRow("Название *:", txtName);

    txtAddress = new QLineEdit(this);
    txtAddress->setPlaceholderText("Адрес филиала");
    formLayout->addRow("Адрес *:", txtAddress);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Сохранить", this);
    QPushButton *btnCancel = new QPushButton("Отмена", this);
    btnCancel->setObjectName("btnCancel");
    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, this, [this]() {
        if (txtName->text().trimmed().isEmpty() || txtAddress->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Предупреждение", "Пожалуйста, заполните все поля.");
            return;
        }
        accept();
    });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void BranchDialog::setBranchData(const QString& name, const QString& address) {
    txtName->setText(name);
    txtAddress->setText(address);
}

void BranchDialog::getBranchData(QString& name, QString& address) const {
    name = txtName->text().trimmed();
    address = txtAddress->text().trimmed();
}

// =========================================================================
// ServiceType Dialog Implementation
// =========================================================================
ServiceTypeDialog::ServiceTypeDialog(QWidget *parent, const QString& title) : QDialog(parent) {
    setWindowTitle(title);
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    txtName = new QLineEdit(this);
    txtName->setPlaceholderText("Название услуги");
    formLayout->addRow("Название *:", txtName);

    txtType = new QLineEdit(this);
    txtType->setPlaceholderText("Категория (например, Аквачистка)");
    formLayout->addRow("Тип услуги *:", txtType);

    sbBaseCost = new QDoubleSpinBox(this);
    sbBaseCost->setRange(0.00, 100000.00);
    sbBaseCost->setValue(1000.00);
    sbBaseCost->setSuffix(" руб.");
    formLayout->addRow("Базовая стоимость *:", sbBaseCost);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Сохранить", this);
    QPushButton *btnCancel = new QPushButton("Отмена", this);
    btnCancel->setObjectName("btnCancel");
    btnLayout->addStretch();
    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnCancel);
    mainLayout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, this, [this]() {
        if (txtName->text().trimmed().isEmpty() || txtType->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Предупреждение", "Пожалуйста, заполните все поля.");
            return;
        }
        accept();
    });
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void ServiceTypeDialog::setServiceData(const QString& name, const QString& type, double baseCost) {
    txtName->setText(name);
    txtType->setText(type);
    sbBaseCost->setValue(baseCost);
}

void ServiceTypeDialog::getServiceData(QString& name, QString& type, double& baseCost) const {
    name = txtName->text().trimmed();
    type = txtType->text().trimmed();
    baseCost = sbBaseCost->value();
}
