#include "authcontroller.h"
#include "../model/database.h"
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSqlError>

AuthController::AuthController(QObject *parent) : QObject(parent) {}

AuthController& AuthController::instance() {
    static AuthController inst;
    return inst;
}

QString AuthController::hashPassword(const QString& password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool AuthController::login(const QString& username, const QString& password) {
    if (!Database::instance().isOpen()) return false;

    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT * FROM users WHERE username = :username AND password_hash = :hash");
    q.bindValue(":username", username);
    q.bindValue(":hash", hashPassword(password));

    if (q.exec() && q.next()) {
        m_currentUser = User::fromQuery(q);
        m_isLoggedIn = true;
        return true;
    }
    return false;
}

bool AuthController::registerUser(const QString& username, const QString& password, const QString& fullName, const QString& role, QString& errorMsg) {
    if (!Database::instance().isOpen()) {
        errorMsg = "База данных не подключена.";
        return false;
    }

    if (username.trimmed().isEmpty() || password.isEmpty() || fullName.trimmed().isEmpty()) {
        errorMsg = "Все поля обязательны для заполнения.";
        return false;
    }

    // Check password strength
    if (!isPasswordStrong(password)) {
        errorMsg = getPasswordRequirementsString();
        return false;
    }

    // Check if user already exists
    QSqlQuery checkQ(Database::instance().db());
    checkQ.prepare("SELECT id FROM users WHERE username = :username");
    checkQ.bindValue(":username", username);
    if (checkQ.exec() && checkQ.next()) {
        errorMsg = "Пользователь с таким именем уже зарегистрирован.";
        return false;
    }

    // Insert user
    QSqlQuery insertQ(Database::instance().db());
    insertQ.prepare("INSERT INTO users (username, password_hash, role, full_name) "
                    "VALUES (:username, :hash, :role, :full_name)");
    insertQ.bindValue(":username", username);
    insertQ.bindValue(":hash", hashPassword(password));
    insertQ.bindValue(":role", role);
    insertQ.bindValue(":full_name", fullName);

    if (!insertQ.exec()) {
        errorMsg = "Ошибка при регистрации: " + insertQ.lastError().text();
        return false;
    }

    return true;
}

bool AuthController::updateUserProfile(int userId, const QString& username, const QString& password, const QString& fullName, QString& errorMsg) {
    if (!Database::instance().isOpen()) {
        errorMsg = "База данных не подключена.";
        return false;
    }

    if (username.trimmed().isEmpty() || fullName.trimmed().isEmpty()) {
        errorMsg = "Логин и ФИО обязательны для заполнения.";
        return false;
    }

    // Check if new username is taken by another user
    QSqlQuery checkQ(Database::instance().db());
    checkQ.prepare("SELECT id FROM users WHERE username = :username AND id != :id");
    checkQ.bindValue(":username", username);
    checkQ.bindValue(":id", userId);
    if (checkQ.exec() && checkQ.next()) {
        errorMsg = "Этот логин уже занят другим пользователем.";
        return false;
    }

    bool changePassword = !password.isEmpty();
    if (changePassword && !isPasswordStrong(password)) {
        errorMsg = getPasswordRequirementsString();
        return false;
    }

    QSqlQuery updateQ(Database::instance().db());
    if (changePassword) {
        updateQ.prepare("UPDATE users SET username = :username, password_hash = :hash, full_name = :full_name WHERE id = :id");
        updateQ.bindValue(":hash", hashPassword(password));
    } else {
        updateQ.prepare("UPDATE users SET username = :username, full_name = :full_name WHERE id = :id");
    }
    updateQ.bindValue(":username", username);
    updateQ.bindValue(":full_name", fullName);
    updateQ.bindValue(":id", userId);

    if (!updateQ.exec()) {
        errorMsg = "Ошибка обновления профиля: " + updateQ.lastError().text();
        return false;
    }

    // Update current session user details if it's the current user
    if (m_currentUser.id == userId) {
        m_currentUser.username = username;
        m_currentUser.fullName = fullName;
        if (changePassword) {
            m_currentUser.passwordHash = hashPassword(password);
        }
    }

    return true;
}

void AuthController::logout() {
    m_currentUser = User();
    m_isLoggedIn = false;
}

bool AuthController::isLoggedIn() const {
    return m_isLoggedIn;
}

User AuthController::currentUser() const {
    return m_currentUser;
}

bool AuthController::isPasswordStrong(const QString& password) {
    // Regex checks:
    // 1. (?=.*\d) - At least one digit
    // 2. (?=.*[A-ZА-Я]) - At least one uppercase letter (Latin or Cyrillic)
    // 3. (?=.*[!@#$%^&*()_+\-=\[\]{};':"\\|,.<>\/?~`\+]) - At least one special char
    // 4. .{8,} - Minimum length of 8 characters
    QRegularExpression regex("^(?=.*\\d)(?=.*[A-ZА-Я])(?=.*[!@#$%^&*()_+\\-=\\[\\]{};':\"\\\\|,.<>\\/?~`\\+]).{8,}$");
    return regex.match(password).hasMatch();
}

QString AuthController::getPasswordRequirementsString() {
    return "Пароль должен удовлетворять требованиям безопасности:\n"
           "- Длина не менее 8 символов;\n"
           "- Содержит как минимум одну цифру;\n"
           "- Содержит как минимум одну заглавную букву (A-Z или А-Я);\n"
           "- Содержит как минимум один специальный символ (например: !, @, #, $, %, *).";
}
