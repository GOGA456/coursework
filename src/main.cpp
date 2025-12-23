#include "../include/mainwindow.h"
#include "../include/logindialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QTextCodec>

bool setupDatabase() {
    // ИЗМЕНЕНО: используем PostgreSQL вместо SQLite
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    
    // Настройки подключения к PostgreSQL
    db.setHostName("localhost");
    db.setDatabaseName("ib_incidents_db");
    db.setUserName("ib_user");
    db.setPassword("ib123456"); 
    
    if (!db.open()) {
        qDebug() << "Cannot open PostgreSQL database:" << db.lastError().text();
        QMessageBox::critical(nullptr, "Ошибка подключения", 
            QString("Не удалось подключиться к PostgreSQL:\n%1\n\n"
                   "Проверьте:\n"
                   "1. Запущен ли PostgreSQL: sudo systemctl status postgresql\n"
                   "2. Существует ли БД: sudo -u postgres psql -c '\\l'\n"
                   "3. Пароль PostgreSQL в коде").arg(db.lastError().text()));
        return false;
    }
    
    qDebug() << "PostgreSQL database opened successfully";
    
    // ПРОВЕРКА: таблицы уже должны существовать
    QSqlQuery checkQuery;
    QStringList tables = {"users", "incidents", "incident_history"};
    
    for (const QString &table : tables) {
        if (!checkQuery.exec(QString("SELECT 1 FROM %1 LIMIT 1").arg(table))) {
            qDebug() << "Table" << table << "doesn't exist or error:" << checkQuery.lastError().text();
            // Таблицы уже должны быть созданы в БД - не создаем их здесь
        } else {
            qDebug() << "Table" << table << "exists";
        }
    }
    
    return true;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    
    if (!setupDatabase()) {
        return 1;
    }
    
    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }
    
    MainWindow mainWindow(
        loginDialog.getUsername(),
        loginDialog.getFullName(),
        loginDialog.getUserRole(),
        loginDialog.getUserId()
    );
    
    mainWindow.show();
    return app.exec();
}
