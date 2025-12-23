#ifndef RUSSIANTABLEMODEL_H
#define RUSSIANTABLEMODEL_H

#include <QSqlQueryModel>
#include <QObject>

class RussianTableModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    explicit RussianTableModel(QObject *parent = nullptr);
    
    // ТОЛЬКО эти два метода - больше ничего!
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    
    // Убраны: setHeaderData и sort - их не должно быть!
};

#endif // RUSSIANTABLEMODEL_H
