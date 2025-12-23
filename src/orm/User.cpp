#include "../include/orm/User.h"
#include <QSqlRecord>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QSqlError>
#include <stdexcept>
#include <QDebug>

User::User(QObject *parent) : BaseModel(parent), m_isActive(true) {
    m_id = -1;
}

QHash<QString, QVariant> User::toHash() const {
    QHash<QString, QVariant> hash;
    if (m_id != -1) hash["id"] = m_id;
    hash["username"] = m_username;
    hash["password_hash"] = m_passwordHash;
    hash["full_name"] = m_fullName;
    hash["role"] = m_role;
    hash["is_active"] = m_isActive;
    
    if (m_id != -1) {
        if (m_createdAt.isValid()) hash["created_at"] = m_createdAt;
        if (m_updatedAt.isValid()) hash["updated_at"] = m_updatedAt;
    }
    
    return hash;
}

void User::fromHash(const QHash<QString, QVariant> &hash) {
    m_id = hash.value("id", -1).toInt();
    m_username = hash.value("username").toString();
    m_passwordHash = hash.value("password_hash").toString();
    m_fullName = hash.value("full_name").toString();
    m_role = hash.value("role", "user").toString();
    m_isActive = hash.value("is_active", true).toBool();
    m_createdAt = hash.value("created_at").toDateTime();
    m_updatedAt = hash.value("updated_at").toDateTime();
}

QStringList User::fields() const {
    return {"id", "username", "password_hash", "full_name", "role", "is_active", "created_at", "updated_at"};
}

QList<User*> User::findAll(const QString &condition) {
    QList<User*> users;
    QString sql = "SELECT * FROM users";
    
    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }
    
    sql += " ORDER BY full_name";
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuerySimple(sql);
    
    for (const auto &row : results) {
        User *user = new User();
        user->fromHash(row);
        users.append(user);
    }
    
    return users;
}

User* User::findById(int id) {
    QString sql = "SELECT * FROM users WHERE id = :id";
    QHash<QString, QVariant> params;
    params["id"] = id;
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    if (!results.isEmpty()) {
        User *user = new User();
        user->fromHash(results.first());
        return user;
    }
    
    return nullptr;
}

User* User::findByUsername(const QString &username) {
    QString sql = "SELECT * FROM users WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    if (!results.isEmpty()) {
        User *user = new User();
        user->fromHash(results.first());
        return user;
    }
    
    return nullptr;
}

QString User::getUserNameById(int userId) {
    if (userId <= 0) return "Не назначен";
    
    QString sql = "SELECT full_name FROM users WHERE id = :id";
    QHash<QString, QVariant> params;
    params["id"] = userId;
    
    QVariant result = BaseModel::executeScalar(sql, params);
    if (result.isValid() && !result.isNull()) {
        return result.toString();
    }
    return QString("User #%1").arg(userId);
}

User* User::findByFullName(const QString &fullName) {
    QString sql = "SELECT * FROM users WHERE full_name = :full_name";
    QHash<QString, QVariant> params;
    params["full_name"] = fullName;
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    if (!results.isEmpty()) {
        User *user = new User();
        user->fromHash(results.first());
        return user;
    }
    
    return nullptr;
}

QList<User*> User::findActiveUsers() {
    return findAll("is_active = true");
}

QList<User*> User::findByRole(const QString &role) {
    return findAll("role = '" + role + "'");
}

bool User::userExists(const QString &username) {
    QString sql = "SELECT COUNT(*) FROM users WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;
    
    QVariant result = BaseModel::executeScalar(sql, params);
    return result.toInt() > 0;
}

int User::count() {
    QString sql = "SELECT COUNT(*) FROM users";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

int User::countActive() {
    QString sql = "SELECT COUNT(*) FROM users WHERE is_active = true";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

void User::setPassword(const QString &password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    m_passwordHash = hash.toHex();
}

bool User::checkPassword(const QString &username, const QString &password) {
    QString sql = "SELECT password_hash FROM users WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;
    
    QVariant result = BaseModel::executeScalar(sql, params);
    if (result.isNull()) {
        return false;
    }
    
    QString storedHash = result.toString();
    QByteArray inputHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString inputHashHex = QString(inputHash.toHex());
    
    return storedHash == inputHashHex;
}

bool User::deleteAll() {
    QString sql = "DELETE FROM users";
    return BaseModel::executeNonQuerySimple(sql);
}

bool User::deactivate(int userId) {
    QString sql = "UPDATE users SET is_active = false WHERE id = :id";
    QHash<QString, QVariant> params;
    params["id"] = userId;
    return BaseModel::executeNonQuery(sql, params);
}

bool User::activate(int userId) {
    QString sql = "UPDATE users SET is_active = true WHERE id = :id";
    QHash<QString, QVariant> params;
    params["id"] = userId;
    return BaseModel::executeNonQuery(sql, params);
}

bool User::updateRole(int userId, const QString &role) {
    QString sql = "UPDATE users SET role = :role WHERE id = :id";
    QHash<QString, QVariant> params;
    params["id"] = userId;
    params["role"] = role;
    return BaseModel::executeNonQuery(sql, params);
}
