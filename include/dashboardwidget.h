#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    void refreshDashboard();

private:
    void setupUI();
    void updateMetrics();
    void loadRecentIncidents();
    void loadStatisticsTable();
    QWidget* createMetricWidget(const QString &title, QLabel *valueLabel);
    
    // Метрики
    QLabel *lblTotalIncidents;
    QLabel *lblActiveIncidents;
    QLabel *lblCriticalIncidents;
    QLabel *lblAvgResolutionTime;
    QLabel *lblSLACompliance;
    QLabel *lblIncidentsToday;
    
    // Таблицы
    QTableWidget *recentIncidentsTable;
    QTableWidget *statsTable;
    
    // Кнопка обновления
    QPushButton *btnRefresh;
};

#endif // DASHBOARDWIDGET_H
