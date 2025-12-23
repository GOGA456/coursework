
#include "../include/dashboardwidget.h"
#include "orm/Incident.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDebug>

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent),
      lblTotalIncidents(nullptr),
      lblActiveIncidents(nullptr),
      lblCriticalIncidents(nullptr),
      lblAvgResolutionTime(nullptr),
      lblSLACompliance(nullptr),
      lblIncidentsToday(nullptr),
      recentIncidentsTable(nullptr),
      statsTable(nullptr),
      btnRefresh(nullptr)
{
    setupUI();
    refreshDashboard();
}

void DashboardWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    
    QWidget *controlPanel = new QWidget();
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    
    btnRefresh = new QPushButton("Обновить");
    
    controlLayout->addStretch();
    controlLayout->addWidget(btnRefresh);
    
    QGroupBox *metricsGroup = new QGroupBox("Ключевые метрики системы");
    QGridLayout *metricsLayout = new QGridLayout(metricsGroup);
    
    lblTotalIncidents = new QLabel("0");
    lblActiveIncidents = new QLabel("0");
    lblCriticalIncidents = new QLabel("0");
    lblAvgResolutionTime = new QLabel("0");
    lblSLACompliance = new QLabel("100%");
    lblIncidentsToday = new QLabel("0");
    
    QFont metricFont;
    metricFont.setPointSize(16);
    metricFont.setBold(true);
    
    lblTotalIncidents->setFont(metricFont);
    lblActiveIncidents->setFont(metricFont);
    lblCriticalIncidents->setFont(metricFont);
    lblAvgResolutionTime->setFont(metricFont);
    lblSLACompliance->setFont(metricFont);
    lblIncidentsToday->setFont(metricFont);
    
    lblTotalIncidents->setStyleSheet("color: #2c3e50;");
    lblActiveIncidents->setStyleSheet("color: #e74c3c;");
    lblCriticalIncidents->setStyleSheet("color: #c0392b;");
    lblAvgResolutionTime->setStyleSheet("color: #3498db;");
    lblSLACompliance->setStyleSheet("color: #27ae60;");
    lblIncidentsToday->setStyleSheet("color: #f39c12;");
    
    metricsLayout->addWidget(createMetricWidget("Всего инцидентов", lblTotalIncidents), 0, 0);
    metricsLayout->addWidget(createMetricWidget("Активных", lblActiveIncidents), 0, 1);
    metricsLayout->addWidget(createMetricWidget("Критические", lblCriticalIncidents), 0, 2);
    metricsLayout->addWidget(createMetricWidget("Среднее время (дни)", lblAvgResolutionTime), 1, 0);
    metricsLayout->addWidget(createMetricWidget("Соответствие SLA", lblSLACompliance), 1, 1);
    metricsLayout->addWidget(createMetricWidget("Сегодня", lblIncidentsToday), 1, 2);
    
    QGroupBox *recentGroup = new QGroupBox("Последние инциденты");
    QVBoxLayout *recentLayout = new QVBoxLayout(recentGroup);
    
    recentIncidentsTable = new QTableWidget();
    recentIncidentsTable->setColumnCount(6);
    recentIncidentsTable->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Заголовок" << "Уровень" << "Статус" << "Сообщил" << "Дата"
    );
    recentIncidentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentIncidentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentIncidentsTable->setAlternatingRowColors(true);
    
    recentIncidentsTable->setColumnWidth(0, 50);
    recentIncidentsTable->setColumnWidth(1, 300);
    recentIncidentsTable->setColumnWidth(2, 100);
    recentIncidentsTable->setColumnWidth(3, 120);
    recentIncidentsTable->setColumnWidth(4, 150);
    recentIncidentsTable->setColumnWidth(5, 150);
    
    recentLayout->addWidget(recentIncidentsTable);
    
    QGroupBox *statsGroup = new QGroupBox("Статистика по статусам и уровням");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
    
    statsTable = new QTableWidget();
    statsTable->setColumnCount(3);
    statsTable->setHorizontalHeaderLabels(
        QStringList() << "Категория" << "Значение" << "Процент"
    );
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setAlternatingRowColors(true);
    
    statsLayout->addWidget(statsTable);
    
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(metricsGroup);
    mainLayout->addWidget(recentGroup);
    mainLayout->addWidget(statsGroup);
    
    connect(btnRefresh, &QPushButton::clicked, this, &DashboardWidget::refreshDashboard);
}

QWidget* DashboardWidget::createMetricWidget(const QString &title, QLabel *valueLabel)
{
    QWidget *container = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(15, 15, 15, 15);
    
    QLabel *titleLabel = new QLabel(title);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #7f8c8d; font-size: 13px; font-weight: bold;");
    
    valueLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    
    container->setStyleSheet(
        "QWidget {"
        "  border: 1px solid #bdc3c7;"
        "  border-radius: 8px;"
        "  background-color: #f8f9fa;"
        "}"
        "QWidget:hover {"
        "  background-color: #ecf0f1;"
        "  border-color: #3498db;"
        "}"
    );
    
    return container;
}

void DashboardWidget::refreshDashboard()
{
    updateMetrics();
    loadRecentIncidents();
    loadStatisticsTable();
}

void DashboardWidget::updateMetrics()
{
    int totalCount = Incident::countTotal();
    int activeCount = Incident::countActive();
    int criticalCount = Incident::countBySeverity("critical");
    int todayCount = Incident::countToday();
    
    lblTotalIncidents->setText(QString::number(totalCount));
    lblActiveIncidents->setText(QString::number(activeCount));
    lblCriticalIncidents->setText(QString::number(criticalCount));
    lblIncidentsToday->setText(QString::number(todayCount));
    
    lblAvgResolutionTime->setText("0");
    lblSLACompliance->setText("100%");
    
    qDebug() << "Dashboard metrics - Total:" << totalCount 
             << "Active:" << activeCount 
             << "Critical:" << criticalCount
             << "Today:" << todayCount;
}

void DashboardWidget::loadRecentIncidents()
{
    if (!recentIncidentsTable) return;
    
    recentIncidentsTable->setRowCount(0);
    
    QList<Incident*> incidents = Incident::findRecent(10);
    
    int count = qMin(10, incidents.count());
    for (int i = 0; i < count; i++) {
        Incident *incident = incidents[i];
        recentIncidentsTable->insertRow(i);
        
        recentIncidentsTable->setItem(i, 0, 
            new QTableWidgetItem(QString::number(incident->getId())));
        
        recentIncidentsTable->setItem(i, 1, 
            new QTableWidgetItem(incident->getTitle()));
        
        QString severity = incident->getSeverity();
        QString severityRu;
        if (severity == "low") severityRu = "Низкий";
        else if (severity == "medium") severityRu = "Средний";
        else if (severity == "high") severityRu = "Высокий";
        else if (severity == "critical") severityRu = "Критический";
        else severityRu = severity;
        
        QTableWidgetItem *severityItem = new QTableWidgetItem(severityRu);
        
        if (severity == "critical") severityItem->setBackground(QColor(231, 76, 60));
        else if (severity == "high") severityItem->setBackground(QColor(230, 126, 34));
        else if (severity == "medium") severityItem->setBackground(QColor(241, 196, 15));
        else severityItem->setBackground(QColor(46, 204, 113));
        
        severityItem->setForeground(Qt::white);
        recentIncidentsTable->setItem(i, 2, severityItem);
        
        QString status = incident->getStatus();
        QString statusRu;
        if (status == "new") statusRu = "Новый";
        else if (status == "investigating") statusRu = "Расследуется";
        else if (status == "contained") statusRu = "Локализован";
        else if (status == "resolved") statusRu = "Решен";
        else if (status == "closed") statusRu = "Закрыт";
        else statusRu = status;
        
        recentIncidentsTable->setItem(i, 3, new QTableWidgetItem(statusRu));
        
        recentIncidentsTable->setItem(i, 4, 
            new QTableWidgetItem(incident->getReporter()));
        
        QDateTime dt = incident->getCreatedDate();
        recentIncidentsTable->setItem(i, 5, 
            new QTableWidgetItem(dt.toString("dd.MM.yyyy HH:mm")));
    }
    
    qDeleteAll(incidents);
}

void DashboardWidget::loadStatisticsTable()
{
    if (!statsTable) return;
    
    statsTable->setRowCount(0);
    
    int totalIncidents = Incident::countTotal();
    
    if (totalIncidents == 0) {
        return;
    }
    
    qDebug() << "Loading statistics for" << totalIncidents << "incidents";
    
    QStringList statuses = {"new", "investigating", "contained", "resolved", "closed"};
    QStringList statusNames = {"Новый", "Расследуется", "Локализован", "Решен", "Закрыт"};
    
    int row = 0;
    
    for (int i = 0; i < statuses.size(); i++) {
        int count = Incident::findByStatus(statuses[i]).count();
        
        if (count > 0) {
            statsTable->insertRow(row);
            statsTable->setItem(row, 0, new QTableWidgetItem("Статус: " + statusNames[i]));
            statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
            
            double percent = (count * 100.0) / totalIncidents;
            statsTable->setItem(row, 2, new QTableWidgetItem(QString("%1%").arg(percent, 0, 'f', 1)));
            row++;
        }
    }
    
    QStringList severities = {"critical", "high", "medium", "low"};
    QStringList severityNames = {"Критический", "Высокий", "Средний", "Низкий"};
    
    for (int i = 0; i < severities.size(); i++) {
        int count = Incident::countBySeverity(severities[i]);
        
        if (count > 0) {
            statsTable->insertRow(row);
            statsTable->setItem(row, 0, new QTableWidgetItem("Уровень: " + severityNames[i]));
            statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
            
            double percent = (count * 100.0) / totalIncidents;
            statsTable->setItem(row, 2, new QTableWidgetItem(QString("%1%").arg(percent, 0, 'f', 1)));
            
            for (int col = 0; col < 3; col++) {
                QTableWidgetItem *item = statsTable->item(row, col);
                if (item) {
                    if (severities[i] == "critical") item->setBackground(QColor(255, 235, 238));
                    else if (severities[i] == "high") item->setBackground(QColor(255, 243, 224));
                    else if (severities[i] == "medium") item->setBackground(QColor(255, 249, 196));
                    else item->setBackground(QColor(232, 245, 233));
                }
            }
            
            qDebug() << "Severity" << severities[i] << ":" << count << "records";
            row++;
        }
    }
    
    statsTable->setColumnWidth(0, 200);
    statsTable->setColumnWidth(1, 100);
    statsTable->setColumnWidth(2, 100);
}

