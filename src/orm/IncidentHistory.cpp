#include "../../include/orm/IncidentHistory.h"
#include "../../include/orm/User.h"
#include <QSqlRecord>
#include <QSqlQuery>

IncidentHistory::IncidentHistory(QObject *parent) : BaseModel(parent), 
    m_incidentId(-1), m_changedById(-1) {
}

QHash<QString, QVariant> IncidentHistory::toHash() const {
    QHash<QString, QVariant> hash;
    if (m_id != -1) hash["id"] = m_id;
    hash["incident_id"] = m_incidentId;
    hash["changed_field"] = m_changedField;
    hash["old_value"] = m_oldValue;
    hash["new_value"] = m_newValue;
    hash["changed_by"] = m_changedBy;
    if (m_changeDate.isValid()) hash["change_date"] = m_changeDate;
    return hash;
}

void IncidentHistory::fromHash(const QHash<QString, QVariant> &hash) {
    m_id = hash.value("id", -1).toInt();
    m_incidentId = hash.value("incident_id", -1).toInt();
    m_changedField = hash.value("changed_field").toString();
    m_oldValue = hash.value("old_value").toString();
    m_newValue = hash.value("new_value").toString();
    m_changedBy = hash.value("changed_by").toString();
    m_changeDate = hash.value("change_date").toDateTime();
}

QStringList IncidentHistory::fields() const {
    return {"id", "incident_id", "changed_field", "old_value", "new_value", 
            "changed_by", "change_date"};
}

QList<IncidentHistory*> IncidentHistory::findByIncidentId(int incidentId) {
    QList<IncidentHistory*> history;
    
    QString sql = "SELECT * FROM incident_history WHERE incident_id = :id ORDER BY change_date DESC";
    QHash<QString, QVariant> params;
    params["id"] = incidentId;
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        IncidentHistory *item = new IncidentHistory();
        item->fromHash(row);
        history.append(item);
    }
    
    return history;
}

QList<IncidentHistory*> IncidentHistory::findByDateRange(const QDate &fromDate, const QDate &toDate) {
    QList<IncidentHistory*> history;
    
    QString sql = "SELECT * FROM incident_history WHERE DATE(change_date) BETWEEN :from_date AND :to_date ORDER BY change_date DESC";
    QHash<QString, QVariant> params;
    params["from_date"] = fromDate.toString("yyyy-MM-dd");
    params["to_date"] = toDate.toString("yyyy-MM-dd");
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        IncidentHistory *item = new IncidentHistory();
        item->fromHash(row);
        history.append(item);
    }
    
    return history;
}

QList<IncidentHistory*> IncidentHistory::findByUser(const QString &username) {
    QList<IncidentHistory*> history;
    
    QString sql = "SELECT * FROM incident_history WHERE changed_by = :username ORDER BY change_date DESC";
    QHash<QString, QVariant> params;
    params["username"] = username;
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        IncidentHistory *item = new IncidentHistory();
        item->fromHash(row);
        history.append(item);
    }
    
    return history;
}

int IncidentHistory::count() {
    QString sql = "SELECT COUNT(*) FROM incident_history";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

int IncidentHistory::countByIncident(int incidentId) {
    QString sql = "SELECT COUNT(*) FROM incident_history WHERE incident_id = :id";
    QHash<QString, QVariant> params;
    params["id"] = incidentId;
    
    QVariant result = BaseModel::executeScalar(sql, params);
    return result.toInt();
}

bool IncidentHistory::clearHistory(int incidentId) {
    QString sql = "DELETE FROM incident_history WHERE incident_id = :id";
    QHash<QString, QVariant> params;
    params["id"] = incidentId;
    
    return BaseModel::executeNonQuery(sql, params);
}

bool IncidentHistory::deleteOldHistory(int days) {
    QString sql = "DELETE FROM incident_history WHERE change_date < NOW() - INTERVAL :days DAY";
    QHash<QString, QVariant> params;
    params["days"] = days;
    
    return BaseModel::executeNonQuery(sql, params);
}

void IncidentHistory::logChange(int incidentId, const QString &field, 
                               const QString &oldValue, const QString &newValue,
                               const QString &changedBy) {
    IncidentHistory history;
    history.setIncidentId(incidentId);
    history.setChangedField(field);
    history.setOldValue(oldValue);
    history.setNewValue(newValue);
    history.setChangedBy(changedBy);
    history.setChangeDate(QDateTime::currentDateTime());
    
    history.save();
}
