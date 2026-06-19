#include "database.h"

Database& Database::instance() {
    static Database inst;
    return inst;
}

Database::~Database() {
    disconnect();
}

bool Database::connect(const QString& host, int port, const QString& dbName, const QString& user, const QString& password) {
    if (m_db.isOpen()) {
        disconnect();
    }
    
    m_db = QSqlDatabase::addDatabase("QPSQL");
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    
    if (!m_db.open()) {
        qDebug() << "Database connection failed:" << m_db.lastError().text();
        return false;
    }
    
    qDebug() << "Database connected successfully.";
    return true;
}

void Database::disconnect() {
    if (m_db.isOpen()) {
        m_db.close();
    }
    // Remove connection to avoid duplicate connection warnings
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

bool Database::isOpen() const {
    return m_db.isOpen();
}

QSqlDatabase Database::db() const {
    return m_db;
}
