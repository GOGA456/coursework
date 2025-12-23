#include "../include/incidenthistorydialog.h"

#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtCore/QDateTime>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QDebug>

IncidentHistoryDialog::IncidentHistoryDialog(int incidentId, const QString &incidentTitle, QWidget *parent)
    : QDialog(parent), incidentId(incidentId), incidentTitle(incidentTitle) {
    
    setWindowTitle("История изменений: " + incidentTitle);
    setMinimumSize(600, 400);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel("История изменений инцидента: <b>" + incidentTitle + "</b>");
    mainLayout->addWidget(titleLabel);
    
    historyBrowser = new QTextBrowser();
    historyBrowser->setReadOnly(true);
    mainLayout->addWidget(historyBrowser, 1);
    
    QPushButton *closeButton = new QPushButton("Закрыть");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    mainLayout->addLayout(buttonLayout);
    
    loadHistory();
}

void IncidentHistoryDialog::loadHistory() {
    if (incidentId <= 0) return;
    
    QString html = "<html><body style='font-family: Arial;'>";
    html += "<h3>История изменений</h3>";
    
    QSqlQuery query;
    query.prepare("SELECT * FROM incident_history WHERE incident_id = :id ORDER BY change_date DESC");
    query.bindValue(":id", incidentId);
    
    if (!query.exec()) {
        html += "<p><i>Ошибка загрузки истории: " + query.lastError().text() + "</i></p>";
    } else {
        if (query.size() == 0) {
            html += "<p><i>История изменений отсутствует</i></p>";
        } else {
            html += "<table border='1' cellpadding='5' style='border-collapse: collapse; width: 100%;'>";
            html += "<tr style='background-color: #f0f0f0;'>";
            html += "<th>Дата</th><th>Поле</th><th>Старое значение</th><th>Новое значение</th><th>Кто изменил</th>";
            html += "</tr>";
            
            while (query.next()) {
                html += "<tr>";
                html += "<td>" + query.value("change_date").toDateTime().toString("dd.MM.yyyy HH:mm") + "</td>";
                html += "<td>" + query.value("changed_field").toString() + "</td>";
                html += "<td>" + query.value("old_value").toString() + "</td>";
                html += "<td>" + query.value("new_value").toString() + "</td>";
                html += "<td>" + query.value("changed_by").toString() + "</td>";
                html += "</tr>";
            }
            
            html += "</table>";
        }
    }
    
    // Загружаем текущее состояние инцидента
    QSqlQuery incidentQuery;
    incidentQuery.prepare("SELECT * FROM incidents WHERE id = :id");
    incidentQuery.bindValue(":id", incidentId);
    
    if (incidentQuery.exec() && incidentQuery.next()) {
        html += "<h3>Текущее состояние</h3>";
        html += "<table border='1' cellpadding='5' style='border-collapse: collapse; width: 100%;'>";
        html += "<tr><td><b>Заголовок:</b></td><td>" + incidentQuery.value("title").toString() + "</td></tr>";
        html += "<tr><td><b>Статус:</b></td><td>" + incidentQuery.value("status").toString() + "</td></tr>";
        html += "<tr><td><b>Уровень угрозы:</b></td><td>" + incidentQuery.value("severity").toString() + "</td></tr>";
        html += "<tr><td><b>Назначен:</b></td><td>" + incidentQuery.value("assigned_to").toString() + "</td></tr>";
        html += "<tr><td><b>Дата создания:</b></td><td>" + incidentQuery.value("created_date").toDateTime().toString("dd.MM.yyyy HH:mm") + "</td></tr>";
        html += "</table>";
    }
    
    html += "</body></html>";
    
    historyBrowser->setHtml(html);
}
