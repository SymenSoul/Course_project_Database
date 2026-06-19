#include "authwindow.h"
#include "../controller/authcontroller.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QApplication>

AuthWindow::AuthWindow(QWidget *parent) : QWidget(parent) {
    setupUI();
    updateUIForMode();
}

void AuthWindow::setupUI() {
    setWindowTitle("Авторизация");
    setFixedSize(400, 500);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // Title
    lblTitle = new QLabel(this);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff; margin-bottom: 20px;");
    mainLayout->addWidget(lblTitle);

    // Form layout
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    txtUsername = new QLineEdit(this);
    txtUsername->setPlaceholderText("Введите ваш логин");
    formLayout->addRow("Логин:", txtUsername);

    txtPassword = new QLineEdit(this);
    txtPassword->setPlaceholderText("Введите ваш пароль");
    txtPassword->setEchoMode(QLineEdit::Password);
    formLayout->addRow("Пароль:", txtPassword);

    txtFullName = new QLineEdit(this);
    txtFullName->setPlaceholderText("ФИО полностью");
    lblFullName = new QLabel("ФИО:");
    formLayout->addRow(lblFullName, txtFullName);

    cbRole = new QComboBox(this);
    cbRole->addItem("Рядовой сотрудник", "Employee");
    cbRole->addItem("Администратор", "Admin");
    lblRole = new QLabel("Роль:");
    formLayout->addRow(lblRole, cbRole);

    mainLayout->addLayout(formLayout);

    // Error label
    lblError = new QLabel(this);
    lblError->setStyleSheet("color: #ef4444; font-size: 12px;");
    lblError->setWordWrap(true);
    lblError->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lblError);

    // Buttons
    btnSubmit = new QPushButton(this);
    mainLayout->addWidget(btnSubmit);

    btnToggle = new QPushButton(this);
    btnToggle->setObjectName("btnSecondary");
    mainLayout->addWidget(btnToggle);

    // Connections
    connect(btnSubmit, &QPushButton::clicked, this, &AuthWindow::handleSubmit);
    connect(btnToggle, &QPushButton::clicked, this, &AuthWindow::toggleMode);
}

void AuthWindow::updateUIForMode() {
    lblError->clear();
    txtPassword->clear();
    
    if (m_isRegisterMode) {
        lblTitle->setText("Регистрация");
        txtFullName->show();
        cbRole->show();
        lblFullName->show();
        lblRole->show();
        btnSubmit->setText("Зарегистрироваться");
        btnToggle->setText("Уже есть аккаунт? Войти");
        setWindowTitle("Регистрация нового пользователя");
    } else {
        lblTitle->setText("Вход в систему");
        txtFullName->hide();
        cbRole->hide();
        lblFullName->hide();
        lblRole->hide();
        btnSubmit->setText("Войти");
        btnToggle->setText("Ещё нет аккаунта? Создать");
        setWindowTitle("Авторизация");
    }
}

void AuthWindow::toggleMode() {
    m_isRegisterMode = !m_isRegisterMode;
    updateUIForMode();
}

void AuthWindow::handleSubmit() {
    lblError->clear();
    QString username = txtUsername->text().trimmed();
    QString password = txtPassword->text();

    if (m_isRegisterMode) {
        QString fullName = txtFullName->text().trimmed();
        QString role = cbRole->currentData().toString();
        QString errorMsg;

        if (AuthController::instance().registerUser(username, password, fullName, role, errorMsg)) {
            QMessageBox::information(this, "Успех", "Регистрация прошла успешно! Теперь вы можете войти.");
            m_isRegisterMode = false;
            updateUIForMode();
            txtUsername->setText(username);
        } else {
            lblError->setText(errorMsg);
        }
    } else {
        if (AuthController::instance().login(username, password)) {
            // Open main application window
            MainWindow *mainWin = new MainWindow();
            mainWin->show();
            this->close();
        } else {
            lblError->setText("Неверный логин или пароль.");
        }
    }
}
