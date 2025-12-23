#ifndef INCIDENTHISTORY_H
#define INCIDENTHISTORY_H

#include "BaseModel.h"
#include <QString>
#include <QDateTime>
#include <QDate>
#include <QList>

class IncidentHistory : public BaseModel
{
    Q_OBJECT
    
public:
    explicit IncidentHistory(QObject *parent = nullptr);
    
    // Геттеры
    int getIncidentId() const { return m_incidentId; }
    QString getChangedField() const { return m_changedField; }
    QString getOldValue() const { return m_oldValue; }
    QString getNewValue() const { return m_newValue; }
    QString getChangedBy() const { return m_changedBy; }
    QDateTime getChangeDate() const { return m_changeDate; }
    int getChangedById() const { return m_changedById; }
    
    // Сеттеры
    void setIncidentId(int id) { m_incidentId = id; }
    void setChangedField(const QString &field) { m_changedField = field; }
    void setOldValue(const QString &value) { m_oldValue = value; }
    void setNewValue(const QString &value) { m_newValue = value; }
    void setChangedBy(const QString &changedBy) { m_changedBy = changedBy; }
    void setChangeDate(const QDateTime &date) { m_changeDate = date; }
    void setChangedById(int id) { m_changedById = id; }
    
    // Унаследованные методы
    QHash<QString, QVariant> toHash() const override;
    void fromHash(const QHash<QString, QVariant> &hash) override;
    QStringList fields() const override;
    QString tableName() const override { return "incident_history"; }
    
    // Статические методы
    static QList<IncidentHistory*> findByIncidentId(int incidentId);
    static QList<IncidentHistory*> findByDateRange(const QDate &fromDate, const QDate &toDate);
    static QList<IncidentHistory*> findByUser(const QString &username);
    static void logChange(int incidentId, const QString &field, 
                         const QString &oldValue, const QString &newValue,
                         const QString &changedBy);
    static int count();
    static int countByIncident(int incidentId);
    static bool clearHistory(int incidentId);
    static bool deleteOldHistory(int days);
    
private:
    int m_incidentId;
    QString m_changedField;
    QString m_oldValue;
    QString m_newValue;
    QString m_changedBy;
    int m_changedById;
    QDateTime m_changeDate;
};

#endif // INCIDENTHISTORY_H
