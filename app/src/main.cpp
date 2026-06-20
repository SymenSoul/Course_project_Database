#include <QApplication>
#include "model/database.h"
#include "view/authwindow.h"
#include <QMessageBox>
#include <QSettings>
#include <QSqlError>
#include <QDir>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Apply the modern dark/indigo stylesheet globally
    a.setStyleSheet(
        "/* Global Dark Theme Stylesheet */\n"
        "QWidget {\n"
        "    background-color: #121214;\n"
        "    color: #e2e2e6;\n"
        "    font-family: 'Outfit', 'Inter', 'Segoe UI', sans-serif;\n"
        "}\n"
        "QMainWindow {\n"
        "    background-color: #121214;\n"
        "}\n"
        "QDialog {\n"
        "    background-color: #1a1a1e;\n"
        "}\n"
        "QLabel {\n"
        "    color: #e2e2e6;\n"
        "    font-size: 13px;\n"
        "}\n"
        "QLineEdit, QComboBox, QDoubleSpinBox, QSpinBox {\n"
        "    background-color: #24242b;\n"
        "    border: 1px solid #3c3c46;\n"
        "    border-radius: 6px;\n"
        "    padding: 6px 12px;\n"
        "    color: #ffffff;\n"
        "    font-size: 13px;\n"
        "}\n"
        "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus {\n"
        "    border: 1px solid #6366f1;\n"
        "}\n"
        "QPushButton {\n"
        "    background-color: #6366f1;\n"
        "    color: #ffffff;\n"
        "    border: none;\n"
        "    border-radius: 6px;\n"
        "    padding: 8px 16px;\n"
        "    font-weight: bold;\n"
        "    font-size: 13px;\n"
        "}\n"
        "QPushButton:hover {\n"
        "    background-color: #4f46e5;\n"
        "}\n"
        "QPushButton:pressed {\n"
        "    background-color: #4338ca;\n"
        "}\n"
        "QPushButton:disabled {\n"
        "    background-color: #3b3b44;\n"
        "    color: #71717a;\n"
        "}\n"
        "QPushButton#btnCancel, QPushButton#btnSecondary {\n"
        "    background-color: #2e2e38;\n"
        "    border: 1px solid #3c3c46;\n"
        "}\n"
        "QPushButton#btnCancel:hover, QPushButton#btnSecondary:hover {\n"
        "    background-color: #3e3e4a;\n"
        "}\n"
        "QPushButton#btnDelete {\n"
        "    background-color: #ef4444;\n"
        "}\n"
        "QPushButton#btnDelete:hover {\n"
        "    background-color: #dc2626;\n"
        "}\n"
        "QTableView {\n"
        "    background-color: #1a1a1e;\n"
        "    alternate-background-color: #202026;\n"
        "    gridline-color: #2e2e38;\n"
        "    color: #e2e2e6;\n"
        "    border: 1px solid #2e2e38;\n"
        "    border-radius: 8px;\n"
        "    font-size: 13px;\n"
        "}\n"
        "QHeaderView::section {\n"
        "    background-color: #26262d;\n"
        "    color: #a1a1aa;\n"
        "    padding: 6px;\n"
        "    border: none;\n"
        "    border-bottom: 1px solid #3c3c46;\n"
        "    font-weight: bold;\n"
        "}\n"
        "QTabBar::tab {\n"
        "    background-color: #1a1a1e;\n"
        "    color: #a1a1aa;\n"
        "    padding: 8px 16px;\n"
        "    border-top-left-radius: 6px;\n"
        "    border-top-right-radius: 6px;\n"
        "    margin-right: 2px;\n"
        "}\n"
        "QTabBar::tab:selected {\n"
        "    background-color: #24242b;\n"
        "    color: #ffffff;\n"
        "    border-bottom: 2px solid #6366f1;\n"
        "}\n"
        "QGroupBox {\n"
        "    border: 1px solid #2e2e38;\n"
        "    border-radius: 8px;\n"
        "    margin-top: 12px;\n"
        "    padding-top: 16px;\n"
        "    font-weight: bold;\n"
        "    color: #ffffff;\n"
        "}\n"
        "QGroupBox::title {\n"
        "    subcontrol-origin: margin;\n"
        "    subcontrol-position: top left;\n"
        "    left: 8px;\n"
        "    padding: 0 4px;\n"
        "}\n"
        "QScrollBar:vertical {\n"
        "    border: none;\n"
        "    background: #1a1a1e;\n"
        "    width: 10px;\n"
        "    margin: 0px 0px 0px 0px;\n"
        "}\n"
        "QScrollBar::handle:vertical {\n"
        "    background: #3e3e4a;\n"
        "    min-height: 20px;\n"
        "    border-radius: 5px;\n"
        "}\n"
        "QScrollBar::handle:vertical:hover {\n"
        "    background: #6366f1;\n"
        "}\n"
    );

    // Load database connection settings from config.ini next to the executable
    QString configPath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/config.ini");
    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("Database");
    if (!settings.contains("Host")) {
        // Write default configuration if it does not exist
        settings.setValue("Host", "127.0.0.1");
        settings.setValue("Port", 5432);
        settings.setValue("DatabaseName", "dryclean");
        settings.setValue("Username", "user");
        settings.setValue("Password", "");
        settings.sync();
    }

    QString host = settings.value("Host", "127.0.0.1").toString();
    int port = settings.value("Port", 5432).toInt();
    QString dbName = settings.value("DatabaseName", "dryclean").toString();
    QString user = settings.value("Username", "user").toString();
    QString password = settings.value("Password", "").toString();
    settings.endGroup();

    // Initialize Database connection (Single Session Connection)
    if (!Database::instance().connect(host, port, dbName, user, password)) {
        QString errorMsg = Database::instance().db().lastError().text();
        QMessageBox::critical(nullptr, "Ошибка БД", 
            "Не удалось подключиться к базе данных PostgreSQL.\n\n"
            "Детали ошибки:\n" + errorMsg + "\n"
            "Пожалуйста, проверьте настройки подключения в файле:\n" + configPath + "\n\n"
            "Убедитесь, что сервер PostgreSQL запущен, указанная база данных создана, а логин и пароль верны.");
        return 1;
    }

    // Launch Authorization screen
    AuthWindow authWin;
    authWin.show();

    return a.exec();
}
