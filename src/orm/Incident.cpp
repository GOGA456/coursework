#include "../../include/orm/Incident.h"
#include "../../include/orm/IncidentHistory.h"
#include <QSqlRecord>
#include <QDate>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

Incident::Incident(QObject *parent) : BaseModel(parent) {
    m_id = -1;
    m_status = "new";
    m_severity = "medium";
    m_createdDate = QDateTime::currentDateTime();
    m_updatedAt = QDateTime::currentDateTime();
    currentFullName = "";
}

void Incident::setStatus(const QString &status) {
    m_status = status;
}

bool Incident::save() {
    QString oldStatus = "new";
    QString oldAssignedTo = "";
    
    if (m_id != -1) {
        QString sql = "SELECT status, assigned_to FROM incidents WHERE id = :id";
        QHash<QString, QVariant> params;
        params["id"] = m_id;
        
        QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
        if (!results.isEmpty()) {
            oldStatus = results.first().value("status").toString();
            oldAssignedTo = results.first().value("assigned_to").toString();
        }
    }
    
    bool result = BaseModel::save();
    
    if (result && m_id != -1) {
        QString changedBy = currentFullName.isEmpty() ? "system" : currentFullName;
        
        if (oldStatus != m_status) {
            IncidentHistory::logChange(m_id, "status", oldStatus, m_status, changedBy);
        }
        
        if (oldAssignedTo != m_assignedTo) {
            IncidentHistory::logChange(m_id, "assigned_to", oldAssignedTo, m_assignedTo, changedBy);
        }
    }
    
    return result;
}

QHash<QString, QVariant> Incident::toHash() const {
    QHash<QString, QVariant> hash;
    
    if (m_id != -1) hash["id"] = m_id;
    hash["title"] = m_title;
    hash["description"] = m_description;
    hash["severity"] = m_severity;
    hash["status"] = m_status;
    hash["reporter"] = m_reporter;
    hash["assigned_to"] = m_assignedTo;
    hash["tags"] = m_tags;
    hash["resolution"] = m_resolution;
    
    if (m_id != -1) {
        if (m_createdDate.isValid()) hash["created_date"] = m_createdDate;
        if (m_createdAt.isValid()) hash["created_at"] = m_createdAt;
        if (m_updatedAt.isValid()) hash["updated_at"] = m_updatedAt;
    }
    
    return hash;
}

void Incident::fromHash(const QHash<QString, QVariant> &hash) {
    m_id = hash.value("id", -1).toInt();
    m_title = hash.value("title").toString();
    m_description = hash.value("description").toString();
    m_severity = hash.value("severity", "medium").toString();
    m_status = hash.value("status", "new").toString();
    m_reporter = hash.value("reporter").toString();
    m_assignedTo = hash.value("assigned_to").toString();
    m_tags = hash.value("tags").toString();
    m_resolution = hash.value("resolution").toString();
    m_createdDate = hash.value("created_date").toDateTime();
    m_createdAt = hash.value("created_at").toDateTime();
    m_updatedAt = hash.value("updated_at").toDateTime();
}

QStringList Incident::fields() const {
    return {"id", "title", "description", "severity", "status", 
            "reporter", "assigned_to", "tags", "resolution", 
            "created_date", "created_at", "updated_at"};
}

QList<Incident*> Incident::findAll(const QString &condition) {
    QList<Incident*> incidents;
    
    qDebug() << "\n=== ВХОД В FINDALL ===";
    qDebug() << "Условие:" << condition;
    
    // СНАЧАЛА проверим что вообще есть в базе с такими параметрами
    if (!condition.isEmpty()) {
        qDebug() << "\nПРОВЕРЯЕМ НАПРЯМУЮ В БАЗЕ:";
        
        QString testSql = "SELECT id, title, status, severity, created_at FROM incidents WHERE " + condition;
        qDebug() << "Тестовый SQL:" << testSql;
        
        QSqlQuery testQuery;
        if (testQuery.exec(testSql)) {
            int count = 0;
            while (testQuery.next()) {
                count++;
                qDebug() << "Найдена запись #" << count << ":";
                qDebug() << "  ID:" << testQuery.value(0).toInt();
                qDebug() << "  Заголовок:" << testQuery.value(1).toString();
                qDebug() << "  Статус:" << testQuery.value(2).toString();
                qDebug() << "  Уровень:" << testQuery.value(3).toString();
                qDebug() << "  Дата:" << testQuery.value(4).toDateTime().toString("yyyy-MM-dd HH:mm:ss");
            }
            qDebug() << "Всего найдено:" << count;
        } else {
            qDebug() << "ОШИБКА SQL:" << testQuery.lastError().text();
        }
    }
    
    // Теперь обычный запрос
    QString sql = "SELECT * FROM incidents";
    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }
    sql += " ORDER BY created_at DESC";
    
    qDebug() << "\nИтоговый SQL:" << sql;
    
    QSqlQuery query;
    if (query.exec(sql)) {
        int count = 0;
        while (query.next()) {
            count++;
            QHash<QString, QVariant> row;
            QSqlRecord record = query.record();
            for (int i = 0; i < record.count(); i++) {
                row[record.fieldName(i)] = record.value(i);
            }
            
            Incident *incident = new Incident();
            incident->fromHash(row);
            incidents.append(incident);
        }
        qDebug() << "Найдено записей:" << count;
    } else {
        qDebug() << "SQL ОШИБКА:" << query.lastError().text();
    }
    
    qDebug() << "=== ВЫХОД ИЗ FINDALL ===\n";
    
    return incidents;
}

Incident* Incident::findById(int id) {
    QString sql = "SELECT * FROM incidents WHERE id = :id";
    QHash<QString, QVariant> params;
    params["id"] = id;
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    if (!results.isEmpty()) {
        Incident *incident = new Incident();
        incident->fromHash(results.first());
        return incident;
    }
    
    return nullptr;
}

QList<Incident*> Incident::search(const QString &text) {
    QList<Incident*> incidents;
    if (text.trimmed().isEmpty()) return findAll();
    
    QString sql = "SELECT * FROM incidents WHERE "
                  "title ILIKE :search OR " 
                  "description ILIKE :search OR "
                  "tags ILIKE :search "
                  "ORDER BY updated_at DESC";
                  
    QHash<QString, QVariant> params;
    params["search"] = "%" + text + "%";
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

QList<Incident*> Incident::findByCondition(const QString &condition) {
    return findAll(condition);
}

int Incident::countActive() {
    QString sql = "SELECT COUNT(*) FROM incidents WHERE status NOT IN ('closed', 'resolved')";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

int Incident::countToday() {
    QString sql = "SELECT COUNT(*) FROM incidents WHERE DATE(created_date) = CURRENT_DATE";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

int Incident::countBySeverity(const QString &severity) {
    QString sql = "SELECT COUNT(*) FROM incidents WHERE severity = :severity";
    QHash<QString, QVariant> params;
    params["severity"] = severity;
    
    QVariant result = BaseModel::executeScalar(sql, params);
    return result.toInt();
}

int Incident::countTotal() {
    QString sql = "SELECT COUNT(*) FROM incidents";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

QList<Incident*> Incident::findByDateRange(const QDate &fromDate, const QDate &toDate) {
    QString sql = "SELECT * FROM incidents WHERE DATE(created_date) BETWEEN :from_date AND :to_date ORDER BY created_date DESC";
    QHash<QString, QVariant> params;
    params["from_date"] = fromDate.toString("yyyy-MM-dd");
    params["to_date"] = toDate.toString("yyyy-MM-dd");
    
    QList<Incident*> incidents;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

QList<Incident*> Incident::findByStatus(const QString &status) {
    return findAll("status = '" + status + "'");
}

QList<Incident*> Incident::findBySeverity(const QString &severity) {
    return findAll("severity = '" + severity + "'");
}

QList<Incident*> Incident::findByAssignedTo(const QString &assignedTo) {
    return findAll("assigned_to = '" + assignedTo + "'");
}

QList<Incident*> Incident::findRecent(int limit) {
    QString sql = QString("SELECT * FROM incidents ORDER BY updated_at DESC LIMIT %1").arg(limit);
    
    QList<Incident*> incidents;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuerySimple(sql);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

bool Incident::updateStatus(const QString &newStatus, const QString &changedBy) {
    if (m_id == -1) return false;
    
    QString oldStatus = m_status;
    m_status = newStatus;
    currentFullName = changedBy;
    
    if (save()) {
        return true;
    }
    return false;
}

void Incident::logStatusChange(const QString &oldStatus, const QString &newStatus, const QString &user) {
    if (m_id != -1 && oldStatus != newStatus) {
        IncidentHistory::logChange(m_id, "status", oldStatus, newStatus, user);
    }
}

QList<IncidentHistory*> Incident::getHistory() const {
    if (m_id == -1) return QList<IncidentHistory*>();
    return IncidentHistory::findByIncidentId(m_id);
}
