#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QStringList>
#include <QList>

class BaseModel : public QObject
{
    Q_OBJECT
    
public:
    explicit BaseModel(QObject *parent = nullptr);
    
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    
    // Чисто виртуальные методы (абстрактные)
    virtual QHash<QString, QVariant> toHash() const = 0;
    virtual void fromHash(const QHash<QString, QVariant> &hash) = 0;
    virtual QStringList fields() const = 0;
    virtual QString tableName() const = 0;
    
    // Виртуальные методы с реализацией по умолчанию
    virtual bool save();
    virtual bool remove();
    
    // Статические утилитные методы для работы с БД
    static QList<QHash<QString, QVariant>> executeQuery(const QString &sql, 
                                                        const QHash<QString, QVariant> &params = {});
    static QList<QHash<QString, QVariant>> executeQuerySimple(const QString &sql);
    static QVariant executeScalar(const QString &sql, const QHash<QString, QVariant> &params = {});
    static QVariant executeScalarSimple(const QString &sql);
    static bool executeNonQuery(const QString &sql, const QHash<QString, QVariant> &params = {});
    static bool executeNonQuerySimple(const QString &sql);
    
    // Методы для резервного копирования и восстановления
    static bool backupDatabase(const QString &filename);
    static bool restoreDatabase(const QString &filename);
    static bool checkTableExists(const QString &tableName);
    static QList<QString> getAllTableNames();
    static QString getDatabaseInfo();
    
    virtual ~BaseModel() {}
    
protected:
    int m_id;
};

#endif // BASEMODEL_H
