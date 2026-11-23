#pragma once

#include <QString>
#include <QVector>
#include <QWidget>

class QPushButton;
class QSqlRelationalTableModel;
class QTabWidget;
class QTableView;

class TableManagementWidget final : public QWidget {
    Q_OBJECT

public:
    explicit TableManagementWidget(const QString &role, QWidget *parent = nullptr);
    void refreshRowHeights();

signals:
    void statusMessageChanged(const QString &message, int timeout);

private slots:
    void addRow();
    void deleteRow();
    void saveChanges();
    void revertChanges();
    void updatePermissions();

private:
    void setupTables();
    QSqlRelationalTableModel *currentModel() const;
    QTableView *currentView() const;

    QString m_role;
    QTabWidget *m_tableTabs = nullptr;
    QVector<QSqlRelationalTableModel *> m_models;
    QVector<QTableView *> m_views;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_revertButton = nullptr;
};
