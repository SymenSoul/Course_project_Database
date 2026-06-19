#ifndef DIALOGS_H
#define DIALOGS_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>

// =========================================================================
// Client Dialog
// =========================================================================
class ClientDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClientDialog(QWidget *parent = nullptr, const QString& title = "Клиент");
    void setClientData(const QString& lastName, const QString& firstName, const QString& middleName, const QString& phone);
    void getClientData(QString& lastName, QString& firstName, QString& middleName, QString& phone) const;

private:
    QLineEdit *txtLastName;
    QLineEdit *txtFirstName;
    QLineEdit *txtMiddleName;
    QLineEdit *txtPhone;
};

// =========================================================================
// Order Dialog
// =========================================================================
class OrderDialog : public QDialog {
    Q_OBJECT
public:
    explicit OrderDialog(QWidget *parent = nullptr);
    int getSelectedBranchId() const;
    int getSelectedClientId() const;

private:
    QComboBox *cbBranch;
    QComboBox *cbClient;
};

// =========================================================================
// OrderPosition Dialog
// =========================================================================
class OrderPositionDialog : public QDialog {
    Q_OBJECT
public:
    explicit OrderPositionDialog(QWidget *parent = nullptr);
    int getSelectedServiceId() const;
    QString getItemName() const;
    double getComplexity() const;
    double getUrgency() const;
    double getWorkVolume() const;

private:
    QComboBox *cbService;
    QLineEdit *txtItemName;
    QComboBox *cbComplexity;
    QComboBox *cbUrgency;
    QDoubleSpinBox *sbVolume;
};

// =========================================================================
// Branch Dialog
// =========================================================================
class BranchDialog : public QDialog {
    Q_OBJECT
public:
    explicit BranchDialog(QWidget *parent = nullptr, const QString& title = "Филиал");
    void setBranchData(const QString& name, const QString& address);
    void getBranchData(QString& name, QString& address) const;

private:
    QLineEdit *txtName;
    QLineEdit *txtAddress;
};

// =========================================================================
// ServiceType Dialog
// =========================================================================
class ServiceTypeDialog : public QDialog {
    Q_OBJECT
public:
    explicit ServiceTypeDialog(QWidget *parent = nullptr, const QString& title = "Вид услуги");
    void setServiceData(const QString& name, const QString& type, double baseCost);
    void getServiceData(QString& name, QString& type, double& baseCost) const;

private:
    QLineEdit *txtName;
    QLineEdit *txtType;
    QDoubleSpinBox *sbBaseCost;
};

#endif // DIALOGS_H
