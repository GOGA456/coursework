#include "incidentdetaildialog.h"
#include "orm/Incident.h"
#include "orm/User.h"
#include "incidenthistorydialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

IncidentDetailDialog::IncidentDetailDialog(int incidentId, bool isAdmin,
                                           const QString &userRole, 
                                           const QString &currentFullName,
                                           int currentUserId,
                                           QWidget *parent)
    : QDialog(parent), m_incidentId(incidentId), m_isAdmin(isAdmin),
      m_userRole(userRole), m_fullName(currentFullName), m_userId(currentUserId) {
    
    m_username = currentFullName; // для совместимости
    
    setWindowTitle(m_incidentId == -1 ? "Новый инцидент" : 
                  QString("Редактирование инцидента #%1").arg(m_incidentId));
    setMinimumSize(600, 500);
    
    setupUI();
    
    if (m_incidentId != -1) {
        loadIncidentData();
    }
}

IncidentDetailDialog::~IncidentDetailDialog() {
}

void IncidentDetailDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QFormLayout *formLayout = new QFormLayout();
    
    titleEdit = new QLineEdit();
    titleEdit->setPlaceholderText("Краткое описание инцидента");
    
    descriptionEdit = new QTextEdit();
    descriptionEdit->setPlaceholderText("Подробное описание...");
    
    severityCombo = new QComboBox();
    severityCombo->addItems(QStringList() << "Низкий" << "Средний" << "Высокий" << "Критический");
    
    statusCombo = new QComboBox();
    statusCombo->addItems(QStringList() << "Новый" << "Расследуется" << "Локализован" << "Решен" << "Закрыт");
    
    reporterEdit = new QLineEdit();
    reporterEdit->setText(m_fullName);
    reporterEdit->setReadOnly(true);
    
    assignedCombo = new QComboBox();
    assignedCombo->addItem("Не назначен", "");
    
    // Загружаем пользователей для выбора
    QList<User*> users = User::findAll("is_active = true");
    for (User *user : users) {
        assignedCombo->addItem(user->getFullName(), user->getFullName());
    }
    
    tagsEdit = new QLineEdit();
    tagsEdit->setPlaceholderText("через запятую: phishing, malware, ddos");
    
    resolutionEdit = new QTextEdit();
    resolutionEdit->setPlaceholderText("Описание решения...");
    
    formLayout->addRow("Заголовок *:", titleEdit);
    formLayout->addRow("Описание:", descriptionEdit);
    formLayout->addRow("Уровень угрозы:", severityCombo);
    formLayout->addRow("Статус:", statusCombo);
    formLayout->addRow("Сообщил:", reporterEdit);
    formLayout->addRow("Назначить:", assignedCombo);
    formLayout->addRow("Теги:", tagsEdit);
    formLayout->addRow("Решение:", resolutionEdit);
    
    mainLayout->addLayout(formLayout);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    saveButton = new QPushButton("Сохранить");
    cancelButton = new QPushButton("Отмена");
    historyButton = new QPushButton("История изменений");
    
    buttonLayout->addWidget(historyButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Показываем историю только для существующих инцидентов
    historyButton->setVisible(m_incidentId != -1);
    
    connect(saveButton, &QPushButton::clicked, this, &IncidentDetailDialog::onSave);
    connect(cancelButton, &QPushButton::clicked, this, &IncidentDetailDialog::onCancel);
    connect(historyButton, &QPushButton::clicked, this, &IncidentDetailDialog::onShowHistory);
}

void IncidentDetailDialog::loadIncidentData() {
    Incident *incident = Incident::findById(m_incidentId);
    if (!incident) {
        QMessageBox::critical(this, "Ошибка", "Инцидент не найден!");
        reject();
        return;
    }
    
    titleEdit->setText(incident->getTitle());
    descriptionEdit->setText(incident->getDescription());
    
    // Устанавливаем уровень угрозы
    QString severity = incident->getSeverity();
    if (severity == "low") severityCombo->setCurrentIndex(0);
    else if (severity == "medium") severityCombo->setCurrentIndex(1);
    else if (severity == "high") severityCombo->setCurrentIndex(2);
    else if (severity == "critical") severityCombo->setCurrentIndex(3);
    
    // Устанавливаем статус
    QString status = incident->getStatus();
    if (status == "new") statusCombo->setCurrentIndex(0);
    else if (status == "investigating") statusCombo->setCurrentIndex(1);
    else if (status == "contained") statusCombo->setCurrentIndex(2);
    else if (status == "resolved") statusCombo->setCurrentIndex(3);
    else if (status == "closed") statusCombo->setCurrentIndex(4);
    
    reporterEdit->setText(incident->getReporter());
    
    // Устанавливаем назначенного
    QString assigned = incident->getAssignedTo();
    for (int i = 0; i < assignedCombo->count(); i++) {
        if (assignedCombo->itemData(i).toString() == assigned) {
            assignedCombo->setCurrentIndex(i);
            break;
        }
    }
    
    tagsEdit->setText(incident->getTags());
    resolutionEdit->setText(incident->getResolution());
    
    delete incident;
}

void IncidentDetailDialog::onSave() {
    QString title = titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заголовок обязателен для заполнения!");
        titleEdit->setFocus();
        return;
    }
    
    Incident incident;
    if (m_incidentId != -1) {
        incident.setId(m_incidentId);
    }
    
    incident.setTitle(title);
    incident.setDescription(descriptionEdit->toPlainText());
    
    // Уровень угрозы
    QString severityText = severityCombo->currentText();
    QString severity;
    if (severityText == "Низкий") severity = "low";
    else if (severityText == "Средний") severity = "medium";
    else if (severityText == "Высокий") severity = "high";
    else if (severityText == "Критический") severity = "critical";
    else severity = "medium";
    incident.setSeverity(severity);
    
    // Статус
    QString statusText = statusCombo->currentText();
    QString status;
    if (statusText == "Новый") status = "new";
    else if (statusText == "Расследуется") status = "investigating";
    else if (statusText == "Локализован") status = "contained";
    else if (statusText == "Решен") status = "resolved";
    else if (statusText == "Закрыт") status = "closed";
    else status = "new";
    incident.setStatus(status);
    
    incident.setReporter(reporterEdit->text());
    incident.setAssignedTo(assignedCombo->currentData().toString());
    incident.setTags(tagsEdit->text());
    incident.setResolution(resolutionEdit->toPlainText());
    
    // Устанавливаем текущего пользователя для логирования истории
    incident.setCurrentUser(m_fullName);
    
    if (incident.save()) {
        QMessageBox::information(this, "Успех", 
            m_incidentId == -1 ? "Инцидент создан!" : "Инцидент обновлен!");
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Ошибка сохранения инцидента!");
    }
}

void IncidentDetailDialog::onCancel() {
    reject();
}

void IncidentDetailDialog::onShowHistory() {
    IncidentHistoryDialog historyDialog(m_incidentId, titleEdit->text(), this);
    historyDialog.exec();
}
