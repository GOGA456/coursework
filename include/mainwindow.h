#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QString>

class QLineEdit;
class QTableView;
class QTabWidget;
class RussianTableModel;
class ManagedFilterPanel;
class DashboardWidget;
class Incident; 

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, const QString &fullName, 
                       const QString &role, int userId, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddIncident();
    void onEditIncident();
    void onDeleteIncident();
    void onRefresh();
    void onSearch();
    void onClearSearch();
    void onTabChanged(int index);
    void onExportData();
    void onBackupData();
    void onRestoreData();
    void onShowAll();
    void onShowActiveOnly();
    void onShowCriticalOnly();
    void onUserManagement();
    
    // ДОБАВЬТЕ ЭТУ СТРОЧКУ:
    void onFilterChanged(const QString &filter);

private:
    void setupUI();
    void loadIncidents();
    void updateIncidentsTable(const QList<Incident*> &incidents);
    void loadDashboard();

    QString currentUsername;
    QString currentFullName;
    QString currentRole;
    int currentUserId;
    
    // UI элементы
    QTabWidget *tabWidget;
    QTableView *incidentsTableView;
    RussianTableModel *incidentsModel;
    QLineEdit *searchEdit;
    ManagedFilterPanel *filterPanel;
    DashboardWidget *dashboardTab;
};

#endif // MAINWINDOW_H
