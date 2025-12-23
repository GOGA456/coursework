#ifndef USER_H
#define USER_H

#include "BaseModel.h"
#include <QString>
#include <QDateTime>
#include <QList>

class User : public BaseModel
{
    Q_OBJECT
    
public:
    explicit User(QObject *parent = nullptr);
    
    // Геттеры
    QString getUsername() const { return m_username; }
    QString getPasswordHash() const { return m_passwordHash; }
    QString getFullName() const { return m_fullName; }
    QString getRole() const { return m_role; }
    bool getIsActive() const { return m_isActive; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    
    // Сеттеры
    void setUsername(const QString &username) { m_username = username; }
    void setPassword(const QString &password);
    void setPasswordHash(const QString &hash) { m_passwordHash = hash; }
    void setFullName(const QString &fullName) { m_fullName = fullName; }
    void setRole(const QString &role) { m_role = role; }
    void setIsActive(bool isActive) { m_isActive = isActive; }
    void setCreatedAt(const QDateTime &date) { m_createdAt = date; }
    void setUpdatedAt(const QDateTime &date) { m_updatedAt = date; }
    
    // Унаследованные методы
    QHash<QString, QVariant> toHash() const override;
    void fromHash(const QHash<QString, QVariant> &hash) override;
    QStringList fields() const override;
    QString tableName() const override { return "users"; }
    
    // Статические методы для работы с пользователями
    static QList<User*> findAll(const QString &condition = "");
    static QList<User*> findActiveUsers();
    static QList<User*> findByRole(const QString &role);
    static User* findById(int id);
    static User* findByUsername(const QString &username);
    static User* findByFullName(const QString &fullName);
    static QString getUserNameById(int userId);
    static bool userExists(const QString &username);
    static bool checkPassword(const QString &username, const QString &password);
    static int count();
    static int countActive();
    static bool deleteAll();
    static bool deactivate(int userId);
    static bool activate(int userId);
    static bool updateRole(int userId, const QString &role);
    
private:
    QString m_username;
    QString m_passwordHash;
    QString m_fullName;
    QString m_role;
    bool m_isActive;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};

#endif // USER_H
