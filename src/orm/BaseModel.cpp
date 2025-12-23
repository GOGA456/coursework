#include "../../include/orm/BaseModel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSqlRecord>
#include <QFile>
#include <QTextStream>

BaseModel::BaseModel(QObject *parent) : QObject(parent), m_id(-1) {
}

QList<QHash<QString, QVariant>> BaseModel::executeQuery(const QString &sql, const QHash<QString, QVariant> &params)
{
    QList<QHash<QString, QVariant>> results;
    QSqlQuery query;
    
    if (!query.prepare(sql)) {
        qDebug() << "SQL Prepare error:" << query.lastError().text() << "\nSQL:" << sql;
        return results;
    }
    
    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
    
    if (!query.exec()) {
        qDebug() << "SQL Execute error:" << query.lastError().text() << "\nSQL:" << sql;
        return results;
    }
    
    while (query.next()) {
        QHash<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            QString field = record.fieldName(i);
            row[field] = query.value(i);
        }
        results.append(row);
    }
    
    return results;
}

QList<QHash<QString, QVariant>> BaseModel::executeQuerySimple(const QString &sql)
{
    QList<QHash<QString, QVariant>> results;
    QSqlQuery query;
    
    if (!query.exec(sql)) {
        qDebug() << "SQL Execute error:" << query.lastError().text() << "\nSQL:" << sql;
        return results;
    }
    
    while (query.next()) {
        QHash<QString, QVariant> row;
        QSqlRecord record = query.record();
        for (int i = 0; i < record.count(); i++) {
            QString field = record.fieldName(i);
            row[field] = query.value(i);
        }
        results.append(row);
    }
    
    return results;
}

QVariant BaseModel::executeScalar(const QString &sql, const QHash<QString, QVariant> &params)
{
    QSqlQuery query;
    
    if (!query.prepare(sql)) {
        qDebug() << "Scalar Prepare error:" << query.lastError().text();
        return QVariant();
    }
    
    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
    
    if (!query.exec()) {
        qDebug() << "Scalar Execute error:" << query.lastError().text();
        return QVariant();
    }
    
    if (query.next()) {
        return query.value(0);
    }
    
    return QVariant();
}

QVariant BaseModel::executeScalarSimple(const QString &sql)
{
    QSqlQuery query;
    
    if (!query.exec(sql)) {
        qDebug() << "Scalar Execute error:" << query.lastError().text();
        return QVariant();
    }
    
    if (query.next()) {
        return query.value(0);
    }
    
    return QVariant();
}

bool BaseModel::executeNonQuery(const QString &sql, const QHash<QString, QVariant> &params)
{
    QSqlQuery query;
    
    if (!query.prepare(sql)) {
        qDebug() << "NonQuery Prepare error:" << query.lastError().text();
        return false;
    }
    
    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
    
    return query.exec();
}

bool BaseModel::executeNonQuerySimple(const QString &sql)
{
    QSqlQuery query;
    return query.exec(sql);
}

bool BaseModel::save() {
    QSqlQuery query;
    QHash<QString, QVariant> data = toHash();
    
    qDebug() << "=== BaseModel::save() ===";
    qDebug() << "Table:" << tableName();
    qDebug() << "ID:" << m_id;
    qDebug() << "Data hash keys:" << data.keys();
    
    if (m_id == -1) {
        QStringList fieldNames;
        QStringList placeholders;
        
        QStringList allFields = fields();
        qDebug() << "All fields:" << allFields;
        
        for (const QString &field : allFields) {
            if (field != "id" && data.contains(field)) {
                fieldNames << field;
                placeholders << ":" + field;
                qDebug() << "  Field to insert:" << field << "=" << data[field];
            }
        }
        
        if (fieldNames.isEmpty()) {
            qDebug() << "No fields to insert!";
            return false;
        }
        
        QString sql = QString("INSERT INTO %1 (%2) VALUES (%3) RETURNING id")
            .arg(tableName())
            .arg(fieldNames.join(", "))
            .arg(placeholders.join(", "));
        
        qDebug() << "INSERT SQL:" << sql;
        
        if (!query.prepare(sql)) {
            qDebug() << "Prepare error:" << query.lastError().text();
            return false;
        }
        
        for (const QString &field : fieldNames) {
            QVariant value = data[field];
            qDebug() << "Binding" << field << "=" << value;
            query.bindValue(":" + field, value);
        }
        
        if (query.exec() && query.next()) {
            m_id = query.value(0).toInt();
            qDebug() << "✓ Insert successful, ID:" << m_id;
            return true;
        } else {
            qDebug() << "✗ Insert error:" << query.lastError().text();
            return false;
        }
    } else {
        QStringList updates;
        QStringList allFields = fields();
        
        for (const QString &field : allFields) {
            if (field != "id" && field != "created_at" && data.contains(field)) {
                if (field != "updated_at") {
                    updates << field + " = :" + field;
                }
            }
        }
        
        if (updates.isEmpty()) {
            qDebug() << "No fields to update!";
            return false;
        }
        
        QString sql = QString("UPDATE %1 SET %2, updated_at = CURRENT_TIMESTAMP WHERE id = :id")
            .arg(tableName())
            .arg(updates.join(", "));
        
        qDebug() << "UPDATE SQL:" << sql;
        
        if (!query.prepare(sql)) {
            qDebug() << "Update prepare error:" << query.lastError().text();
            return false;
        }
        
        query.bindValue(":id", m_id);
        qDebug() << "Binding id =" << m_id;
        
        for (const QString &field : allFields) {
            if (field != "id" && field != "created_at" && field != "updated_at" && data.contains(field)) {
                QVariant value = data[field];
                qDebug() << "Binding" << field << "=" << value;
                query.bindValue(":" + field, value);
            }
        }
        
        bool result = query.exec();
        if (!result) {
            qDebug() << "✗ Update exec error:" << query.lastError().text();
        } else {
            qDebug() << "✓ Update successful, rows affected:" << query.numRowsAffected();
        }
        return result;
    }
    
    return false;
}

bool BaseModel::remove() {
    if (m_id == -1) {
        return false;
    }
    
    QSqlQuery query;
    query.prepare(QString("DELETE FROM %1 WHERE id = :id").arg(tableName()));
    query.bindValue(":id", m_id);
    
    return query.exec();
}

bool BaseModel::backupDatabase(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    
    QList<QString> tables = getAllTableNames();
    
    for (const QString &table : tables) {
        QString sql = QString("SELECT * FROM %1").arg(table);
        QList<QHash<QString, QVariant>> results = executeQuerySimple(sql);
        
        for (const auto &row : results) {
            stream << "INSERT INTO " << table << " (";
            
            QStringList columns;
            QStringList values;
            
            for (auto it = row.begin(); it != row.end(); ++it) {
                columns << it.key();
                QString value = it.value().toString();
                value.replace("'", "''");
                values << "'" + value + "'";
            }
            
            stream << columns.join(", ") << ") VALUES (" << values.join(", ") << ");\n";
        }
    }
    
    file.close();
    return true;
}

bool BaseModel::restoreDatabase(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    
    QList<QString> tables = getAllTableNames();
    for (const QString &table : tables) {
        QString sql = QString("DELETE FROM %1").arg(table);
        executeNonQuerySimple(sql);
    }
    
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.startsWith("INSERT") || line.startsWith("insert")) {
            if (!executeNonQuerySimple(line)) {
                qDebug() << "Restore error for SQL:" << line;
                return false;
            }
        }
    }
    
    file.close();
    return true;
}

bool BaseModel::checkTableExists(const QString &tableName) {
    QString sql = "SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_name = :table_name)";
    QHash<QString, QVariant> params;
    params["table_name"] = tableName;
    
    QVariant result = executeScalar(sql, params);
    return result.isValid() && result.toBool();
}

QList<QString> BaseModel::getAllTableNames() {
    QList<QString> tables;
    
    QString sql = "SELECT table_name FROM information_schema.tables "
                  "WHERE table_schema = 'public' AND table_type = 'BASE TABLE' "
                  "ORDER BY table_name";
    
    QList<QHash<QString, QVariant>> results = executeQuerySimple(sql);
    for (const auto &row : results) {
        tables.append(row.value("table_name").toString());
    }
    
    return tables;
}

QString BaseModel::getDatabaseInfo() {
    QString info;
    
    QString sql = "SELECT version()";
    QVariant version = executeScalarSimple(sql);
    info += "PostgreSQL Version: " + version.toString() + "\n\n";
    
    QList<QString> tables = getAllTableNames();
    info += "Tables in database (" + QString::number(tables.size()) + "):\n";
    for (const QString &table : tables) {
        QString countSql = QString("SELECT COUNT(*) FROM %1").arg(table);
        QVariant count = executeScalarSimple(countSql);
        info += "  " + table + ": " + count.toString() + " records\n";
    }
    
    return info;
}
