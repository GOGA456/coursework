#include "mainwindow.h"
#include "orm/Incident.h"
#include "orm/User.h"
#include "incidentdetaildialog.h"
#include "russiantablemodel.h"
#include "modules/usermanagement.h"
#include "modules/exportmodule.h"
#include "modules/managedfilterpanel.h"
#include "dashboardwidget.h" 
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QSqlRecord>
#include <QTabWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QDateEdit>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QToolButton>
#include <QProgressBar>
#include <QRadioButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QButtonGroup>

MainWindow::MainWindow(const QString &username, const QString &fullName, 
                       const QString &role, int userId, QWidget *parent)
    : QWidget(parent), currentUsername(username), currentFullName(fullName), 
      currentRole(role), currentUserId(userId) {
    
    filterPanel = nullptr;
    
    setWindowTitle(QString("Система управления ИБ | %1 [%2]").arg(fullName).arg(role));
    setMinimumSize(1200, 600);
    
    setupUI();
    loadIncidents();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Меню
    QMenuBar *menuBar = new QMenuBar(this);
    
    // Меню Файл
    QMenu *fileMenu = menuBar->addMenu("Файл");
    QAction *exportAction = fileMenu->addAction("Экспорт данных");
    QAction *backupAction = fileMenu->addAction("Резервное копирование");
    QAction *restoreAction = fileMenu->addAction("Восстановление");
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("Выход из системы");
    
    // Меню Вид
    QMenu *viewMenu = menuBar->addMenu("Вид");
    QAction *showAllAction = viewMenu->addAction("Показать все");
    QAction *showActiveOnlyAction = viewMenu->addAction("Только активные");
    QAction *showCriticalOnlyAction = viewMenu->addAction("Только критические");
    
    // Меню Инструменты
    QMenu *toolsMenu = menuBar->addMenu("Инструменты");
    QAction *userManagementAction = toolsMenu->addAction("Управление пользователями");
    
    // Подключаем меню
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportData);
    connect(backupAction, &QAction::triggered, this, &MainWindow::onBackupData);
    connect(restoreAction, &QAction::triggered, this, &MainWindow::onRestoreData);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    connect(showAllAction, &QAction::triggered, this, &MainWindow::onShowAll);
    connect(showActiveOnlyAction, &QAction::triggered, this, &MainWindow::onShowActiveOnly);
    connect(showCriticalOnlyAction, &QAction::triggered, this, &MainWindow::onShowCriticalOnly);
    
    connect(userManagementAction, &QAction::triggered, this, &MainWindow::onUserManagement);
    
    mainLayout->setMenuBar(menuBar);
    
    // ========== ВКЛАДКИ ==========
    tabWidget = new QTabWidget();
    
    // ========== ВКЛАДКА 1: ИНЦИДЕНТЫ ==========
    QWidget *incidentsTab = new QWidget();
    QVBoxLayout *incidentsLayout = new QVBoxLayout(incidentsTab);
    
    // Панель информации о пользователе
    QWidget *userPanel = new QWidget();
    QHBoxLayout *userLayout = new QHBoxLayout(userPanel);
    QLabel *userInfo = new QLabel(
        QString("Пользователь: <b>%1</b> | Роль: <b>%2</b>").arg(currentFullName).arg(currentRole));
    userLayout->addWidget(userInfo);
    userLayout->addStretch();
    
    // Панель быстрых действий
    QWidget *actionsPanel = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsPanel);
    
    QPushButton *btnAdd = new QPushButton("Новый инцидент");
    QPushButton *btnEdit = new QPushButton("Редактировать");
    QPushButton *btnDelete = new QPushButton("Удалить");
    QPushButton *btnRefresh = new QPushButton("Обновить");
    
    actionsLayout->addWidget(btnAdd);
    actionsLayout->addWidget(btnEdit);
    actionsLayout->addWidget(btnDelete);
    actionsLayout->addStretch();
    actionsLayout->addWidget(btnRefresh);
    
    // ========== РАСШИРЕННЫЙ ПОИСК ==========
    filterPanel = new ManagedFilterPanel();
    connect(filterPanel, &ManagedFilterPanel::filterChanged, this, &MainWindow::loadIncidents);
    
    // Быстрый поиск отдельно
    QWidget *quickSearchPanel = new QWidget();
    QHBoxLayout *quickSearchLayout = new QHBoxLayout(quickSearchPanel);
    
    QLabel *quickSearchLabel = new QLabel("Быстрый поиск:");
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск по заголовку, описанию или тегам...");
    QPushButton *btnSearch = new QPushButton("Найти");
    QPushButton *btnClear = new QPushButton("Очистить");
    
    quickSearchLayout->addWidget(quickSearchLabel);
    quickSearchLayout->addWidget(searchEdit);
    quickSearchLayout->addWidget(btnSearch);
    quickSearchLayout->addWidget(btnClear);
    
    // Таблица инцидентов
    incidentsTableView = new QTableView();
    incidentsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    incidentsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    incidentsTableView->setAlternatingRowColors(true);
    incidentsTableView->setSortingEnabled(true);
    
    incidentsModel = new RussianTableModel(this);
    incidentsTableView->setModel(incidentsModel);
    
    // Настраиваем ширину столбцов
    incidentsTableView->setColumnWidth(0, 50);
    incidentsTableView->setColumnWidth(1, 250);
    incidentsTableView->setColumnWidth(2, 300);
    incidentsTableView->setColumnWidth(3, 100);
    incidentsTableView->setColumnWidth(4, 120);
    incidentsTableView->setColumnWidth(5, 120);
    incidentsTableView->setColumnWidth(6, 120);
    incidentsTableView->setColumnWidth(7, 150);
    incidentsTableView->setColumnWidth(8, 200);
    incidentsTableView->setColumnWidth(9, 150);
    
    // Собираем вкладку инцидентов
    incidentsLayout->addWidget(userPanel);
    incidentsLayout->addWidget(actionsPanel);
    incidentsLayout->addWidget(filterPanel);
    incidentsLayout->addWidget(quickSearchPanel);
    incidentsLayout->addWidget(incidentsTableView, 1);
    
    // ========== ВКЛАДКА 2: ДАШБОРД ==========
    dashboardTab = new DashboardWidget();
    
    // Добавляем вкладки
    tabWidget->addTab(incidentsTab, "Инциденты");
    tabWidget->addTab(dashboardTab, "Дашборд");
    
    mainLayout->addWidget(tabWidget);
    
    // Подключаем сигналы
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAddIncident);
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEditIncident);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDeleteIncident);
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefresh);
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClearSearch);
    connect(searchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearch);
    connect(incidentsTableView, &QTableView::doubleClicked, this, &MainWindow::onEditIncident);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

void MainWindow::loadIncidents() {
    QList<Incident*> incidents;
    
    // 1. Быстрый поиск
    QString searchText = searchEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        incidents = Incident::search(searchText);
    }
    // 2. Расширенный фильтр
    else if (filterPanel && filterPanel->isExpanded()) {
        QString filter = filterPanel->getFilterString();
        qDebug() << "Используется расширенный фильтр:" << filter;
        
        if (filter.isEmpty()) {
            incidents = Incident::findAll(); // Показать все
        } else {
            incidents = Incident::findAll(filter);
        }
    }
    // 3. Показать все
    else {
        incidents = Incident::findAll();
    }
    
    updateIncidentsTable(incidents);
    
    for (auto incident : incidents) {
        delete incident;
    }
}

void MainWindow::onFilterChanged(const QString &filter) {
    qDebug() << "Filter changed signal received:" << filter;
    loadIncidents();
}


void MainWindow::updateIncidentsTable(const QList<Incident*> &incidents) {
    QStandardItemModel *model = new QStandardItemModel(incidents.size(), 10, this);
    
    QStringList headers = {"ID", "Заголовок", "Описание", "Уровень угрозы", 
                          "Статус", "Сообщил", "Назначен", "Теги", 
                          "Решение", "Дата создания"};
    model->setHorizontalHeaderLabels(headers);
    
    for (int i = 0; i < incidents.size(); i++) {
        Incident *incident = incidents[i];
        
        model->setItem(i, 0, new QStandardItem(QString::number(incident->getId())));
        model->setItem(i, 1, new QStandardItem(incident->getTitle()));
        model->setItem(i, 2, new QStandardItem(incident->getDescription()));
        
        QString severity = incident->getSeverity();
        QString severityRu;
        if (severity == "low") severityRu = "Низкий";
        else if (severity == "medium") severityRu = "Средний";
        else if (severity == "high") severityRu = "Высокий";
        else if (severity == "critical") severityRu = "Критический";
        else severityRu = severity;
        model->setItem(i, 3, new QStandardItem(severityRu));
        
        QString status = incident->getStatus();
        QString statusRu;
        if (status == "new") statusRu = "Новый";
        else if (status == "investigating") statusRu = "Расследуется";
        else if (status == "contained") statusRu = "Локализован";
        else if (status == "resolved") statusRu = "Решен";
        else if (status == "closed") statusRu = "Закрыт";
        else statusRu = status;
        model->setItem(i, 4, new QStandardItem(statusRu));
        
        model->setItem(i, 5, new QStandardItem(incident->getReporter()));
        model->setItem(i, 6, new QStandardItem(incident->getAssignedTo()));
        model->setItem(i, 7, new QStandardItem(incident->getTags()));
        model->setItem(i, 8, new QStandardItem(incident->getResolution()));
        
        QDateTime dt = incident->getCreatedDate();
        model->setItem(i, 9, new QStandardItem(dt.toString("dd.MM.yyyy HH:mm")));
    }
    
    // Удаляем старую модель если она есть и не является incidentsModel
    QAbstractItemModel *oldModel = incidentsTableView->model();
    if (oldModel && oldModel != incidentsModel) {
        delete oldModel;
    }
    
    incidentsTableView->setModel(model);
}

void MainWindow::onAddIncident() {
    IncidentDetailDialog dialog(-1, currentRole == "admin", currentRole, 
                               currentFullName, currentUserId, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadIncidents();
    }
}

void MainWindow::onEditIncident() {
    QModelIndexList selection = incidentsTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Выберите инцидент для редактирования!");
        return;
    }
    
    int row = selection.first().row();
    QAbstractItemModel *model = incidentsTableView->model();
    int incidentId = model->data(model->index(row, 0)).toInt();
    
    IncidentDetailDialog dialog(incidentId, currentRole == "admin", currentRole, 
                               currentFullName, currentUserId, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadIncidents();
    }
}

void MainWindow::onDeleteIncident() {
    QModelIndexList selection = incidentsTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Выберите инцидент для удаления!");
        return;
    }
    
    int row = selection.first().row();
    QAbstractItemModel *model = incidentsTableView->model();
    int incidentId = model->data(model->index(row, 0)).toInt();
    QString title = model->data(model->index(row, 1)).toString();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение", 
                                 QString("Удалить инцидент:\n\"%1\"?").arg(title),
                                 QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        Incident *incident = Incident::findById(incidentId);
        if (incident && incident->remove()) {
            QMessageBox::information(this, "Успех", "Инцидент удален!");
            loadIncidents();
        } else {
            QMessageBox::critical(this, "Ошибка", "Ошибка при удалении!");
        }
        delete incident;
    }
}

void MainWindow::onRefresh() {
    loadIncidents();
}

void MainWindow::onSearch() {
    loadIncidents();
}

void MainWindow::onClearSearch() {
    searchEdit->clear();
    loadIncidents();
}

void MainWindow::onTabChanged(int index) {
    if (index == 1) {
        loadDashboard();
    }
}

void MainWindow::loadDashboard() {
    if (dashboardTab) {
        dashboardTab->refreshDashboard();
    }
}

void MainWindow::onExportData() {

    QAbstractItemModel *currentModel = incidentsTableView->model();
    if (!currentModel) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для экспорта!");
        return;
    }
    
    ExportDialog exportDialog(currentModel, this);
    exportDialog.exec();
}

void MainWindow::onBackupData() {
    QString fileName = QFileDialog::getSaveFileName(this, "Резервное копирование БД", 
                                                   "ib_backup_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".sql", 
                                                   "SQL Files (*.sql);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    if (BaseModel::backupDatabase(fileName)) {
        QMessageBox::information(this, "Успех", "Резервная копия создана!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Ошибка создания резервной копии!");
    }
}

void MainWindow::onRestoreData() {
    QString fileName = QFileDialog::getOpenFileName(this, "Восстановление БД", 
                                                   "", "SQL Files (*.sql);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Предупреждение", 
                                "Восстановление БД удалит все текущие данные!\nПродолжить?",
                                QMessageBox::Yes | QMessageBox::No);
    
    if (reply != QMessageBox::Yes) return;
    
    if (BaseModel::restoreDatabase(fileName)) {
        QMessageBox::information(this, "Успех", "База данных восстановлена!");
        loadIncidents();
    } else {
        QMessageBox::critical(this, "Ошибка", "Ошибка восстановления базы данных!");
    }
}

void MainWindow::onShowAll() {
    searchEdit->clear();
    if (filterPanel) {
        filterPanel->resetFilters();
    }
    loadIncidents();
}

void MainWindow::onShowActiveOnly() {
    searchEdit->clear();
    loadIncidents();
}

void MainWindow::onShowCriticalOnly() {
    searchEdit->clear();
    loadIncidents();
}

void MainWindow::onUserManagement() {
    UserManagementPanel *userPanel = new UserManagementPanel();
    userPanel->setAttribute(Qt::WA_DeleteOnClose);
    userPanel->setWindowModality(Qt::ApplicationModal);
    userPanel->resize(800, 600);
    userPanel->setWindowTitle("Управление пользователями");
    userPanel->show();
}
