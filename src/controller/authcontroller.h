#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <QObject>
#include "../model/entities.h"

class AuthController : public QObject {
    Q_OBJECT
public:
    explicit AuthController(QObject *parent = nullptr);
    static AuthController& instance();

    bool login(const QString& username, const QString& password);
    bool registerUser(const QString& username, const QString& password, const QString& fullName, const QString& role, QString& errorMsg);
    bool updateUserProfile(int userId, const QString& username, const QString& password, const QString& fullName, QString& errorMsg);
    
    void logout();
    bool isLoggedIn() const;
    User currentUser() const;
    
    // Password complexity validation using QRegularExpression
    static bool isPasswordStrong(const QString& password);
    static QString getPasswordRequirementsString();
    static QString hashPassword(const QString& password);

private:
    User m_currentUser;
    bool m_isLoggedIn = false;
};

#endif // AUTHCONTROLLER_H
