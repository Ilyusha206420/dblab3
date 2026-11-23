#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QSqlQueryModel;
class QTableView;

class ReportsWidget final : public QWidget {
    Q_OBJECT

public:
    explicit ReportsWidget(QWidget *parent = nullptr);

private slots:
    void runReport();

private:
    QComboBox *m_reportSelector = nullptr;
    QLineEdit *m_filterEdit = nullptr;
    QTableView *m_reportView = nullptr;
    QSqlQueryModel *m_reportModel = nullptr;
};
