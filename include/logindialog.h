#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>

class LoginDialog : public QDialog {
    Q_OBJECT
    
private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    int userId;
    QString fullName;
    QString userRole;
    QString username;  // Добавьте эту переменную
    
public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    
    int getUserId() const { return userId; }
    QString getFullName() const { return fullName; }
    QString getUserRole() const { return userRole; }
    QString getUsername() const { return username; }  // Добавьте этот метод
    
private slots:
    void setupUI();
    void onLogin();
};

#endif
