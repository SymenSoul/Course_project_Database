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

