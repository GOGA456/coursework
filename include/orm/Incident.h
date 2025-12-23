#ifndef INCIDENT_H
#define INCIDENT_H

#include "BaseModel.h"
#include <QString>
#include <QDateTime>
#include <QList>
#include <QDate>

class IncidentHistory;

class Incident : public BaseModel
{
    Q_OBJECT
    
public:
    explicit Incident(QObject *parent = nullptr);
    
    // Геттеры
    QString getTitle() const { return m_title; }
    QString getDescription() const { return m_description; }
    QString getSeverity() const { return m_severity; }
    QString getStatus() const { return m_status; }
    QString getReporter() const { return m_reporter; }
    QString getAssignedTo() const { return m_assignedTo; }
    QString getTags() const { return m_tags; }
    QString getResolution() const { return m_resolution; }
    QDateTime getCreatedDate() const { return m_createdDate; }
    QDateTime getCreatedAt() const { return m_createdAt; }
    QDateTime getUpdatedAt() const { return m_updatedAt; }
    
    // Сеттеры
    void setTitle(const QString &title) { m_title = title; }
    void setDescription(const QString &description) { m_description = description; }
    void setSeverity(const QString &severity) { m_severity = severity; }
    void setStatus(const QString &status);
    void setReporter(const QString &reporter) { m_reporter = reporter; }
    void setAssignedTo(const QString &assignedTo) { m_assignedTo = assignedTo; }
    void setTags(const QString &tags) { m_tags = tags; }
    void setResolution(const QString &resolution) { m_resolution = resolution; }
    void setCreatedDate(const QDateTime &date) { m_createdDate = date; }
    void setCreatedAt(const QDateTime &date) { m_createdAt = date; }
    void setUpdatedAt(const QDateTime &date) { m_updatedAt = date; }
    
    // Метод для установки текущего пользователя для логирования истории
    void setCurrentUser(const QString &user) { currentFullName = user; }
    
    // Унаследованные методы
    QHash<QString, QVariant> toHash() const override;
    void fromHash(const QHash<QString, QVariant> &hash) override;
    QStringList fields() const override;
    QString tableName() const override { return "incidents"; }
    bool save();  // Переопределение для логирования истории
    
    // Статические методы
    static QList<Incident*> findAll(const QString &condition = "");
    static Incident* findById(int id);
    static QList<Incident*> search(const QString &text);
    static QList<Incident*> findByCondition(const QString &condition);
    static int countActive();
    static int countToday();
    static int countBySeverity(const QString &severity);
    static int countTotal();
    static QList<Incident*> findByDateRange(const QDate &fromDate, const QDate &toDate);
    static QList<Incident*> findByStatus(const QString &status);
    static QList<Incident*> findBySeverity(const QString &severity);
    static QList<Incident*> findByAssignedTo(const QString &assignedTo);
    static QList<Incident*> findRecent(int limit = 10);
    
    // Методы работы с историей
    QList<IncidentHistory*> getHistory() const;
    bool updateStatus(const QString &newStatus, const QString &changedBy);
    void logStatusChange(const QString &oldStatus, const QString &newStatus, const QString &user);
    
private:
    QString m_title;
    QString m_description;
    QString m_severity;
    QString m_status;
    QString m_reporter;
    QString m_assignedTo;
    QString m_tags;
    QString m_resolution;
    QDateTime m_createdDate;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
    QString currentFullName;  // для логирования истории изменений
};

#endif // INCIDENT_H
