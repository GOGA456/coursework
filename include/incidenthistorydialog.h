#ifndef INCIDENTHISTORYDIALOG_H
#define INCIDENTHISTORYDIALOG_H

#include <QtWidgets/QDialog>

class QTextBrowser;

class IncidentHistoryDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit IncidentHistoryDialog(int incidentId, const QString &incidentTitle, QWidget *parent = nullptr);
    
private:
    void loadHistory();
    
    int incidentId;
    QString incidentTitle;
    QTextBrowser *historyBrowser;
};

#endif // INCIDENTHISTORYDIALOG_H
