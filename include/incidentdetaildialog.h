#ifndef INCIDENTDETAILDIALOG_H
#define INCIDENTDETAILDIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QTextEdit;
class QComboBox;
class QPushButton;

class IncidentDetailDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit IncidentDetailDialog(int incidentId = -1, bool isAdmin = false,
                                 const QString &userRole = "user", 
                                 const QString &currentFullName = "",
                                 int currentUserId = -1,
                                 QWidget *parent = nullptr);
    ~IncidentDetailDialog();
    
    QString getUsername() const { return m_username; }
    QString getFullName() const { return m_fullName; }
    QString getUserRole() const { return m_userRole; }
    int getUserId() const { return m_userId; }
    
private slots:
    void onSave();
    void onCancel();
    void onShowHistory();
    
private:
    void setupUI();
    void loadIncidentData();
    
    int m_incidentId;
    bool m_isAdmin;
    QString m_userRole;
    QString m_username;
    QString m_fullName;
    int m_userId;
    
    // UI элементы
    QLineEdit *titleEdit;
    QTextEdit *descriptionEdit;
    QComboBox *severityCombo;
    QComboBox *statusCombo;
    QLineEdit *reporterEdit;
    QComboBox *assignedCombo;
    QLineEdit *tagsEdit;
    QTextEdit *resolutionEdit;
    QPushButton *saveButton;
    QPushButton *cancelButton;
    QPushButton *historyButton;
};

#endif // INCIDENTDETAILDIALOG_H
