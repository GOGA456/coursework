#ifndef EXPORTMODULE_H
#define EXPORTMODULE_H

#include <QDialog>
#include <QAbstractItemModel>
#include <QGridLayout>
#include <QList>

#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QAbstractItemModel *dataModel, QWidget *parent = nullptr);

private slots:
    void onBrowse();
    void onFormatChanged(int index);
    void onSelectAllColumns();
    void onDeselectAllColumns();
    void onExport();

private:
    void setupUI();
    void setupColumnCheckboxes();
    
    QList<int> getSelectedColumns() const;
    QString translateValue(const QString &value, const QString &header) const;
    bool exportToCSV(const QString &filename);
    bool exportToPDF(const QString &filename, bool isPreview = false);
    
    QAbstractItemModel *model;
    
    // UI элементы
    QLineEdit *filenameEdit;
    QComboBox *formatCombo;
    QPushButton *browseButton;
    
    QDateEdit *dateFromEdit;
    QDateEdit *dateToEdit;
    QComboBox *severityCombo;
    QComboBox *statusCombo;
    QLineEdit *searchFilter;
    
    QList<QCheckBox*> columnCheckboxes;
    QGridLayout *columnsGridLayout;
    QPushButton *selectAllButton;
    QPushButton *deselectAllButton;
    
    QCheckBox *includeHeaderCheck;
    QCheckBox *translateDataCheck;
    QCheckBox *openAfterExportCheck;
    
    QGroupBox *filterGroup;
    QGroupBox *columnsGroup;
};

#endif // EXPORTMODULE_H
