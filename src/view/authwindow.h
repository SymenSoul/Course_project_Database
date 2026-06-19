#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

class AuthWindow : public QWidget {
    Q_OBJECT
public:
    explicit AuthWindow(QWidget *parent = nullptr);

private slots:
    void handleSubmit();
    void toggleMode();

private:
    void setupUI();
    void updateUIForMode();

    bool m_isRegisterMode = false;

    QLabel *lblTitle;
    QLineEdit *txtUsername;
    QLineEdit *txtPassword;
    QLineEdit *txtFullName;
    QComboBox *cbRole;
    QPushButton *btnSubmit;
    QPushButton *btnToggle;
    QLabel *lblError;

    QLabel *lblFullName;
    QLabel *lblRole;
};

#endif // AUTHWINDOW_H
