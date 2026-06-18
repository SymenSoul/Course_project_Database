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

