#include "ReportsWidget.h"

#include <QAbstractItemView>
#include <QColor>
#include <QComboBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>
#include <QPushButton>
#include <QSize>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QVBoxLayout>
#include <utility>

ReportsWidget::ReportsWidget(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);

    m_reportSelector = new QComboBox(this);
    m_reportSelector->setMinimumContentsLength(26);
    m_reportSelector->setSizeAdjustPolicy(
        QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_reportSelector->setStyleSheet(
        "QComboBox { color: palette(text); background-color: palette(base); }"
        "QComboBox QAbstractItemView { color: palette(text); "
        "background-color: palette(base); "
        "selection-color: palette(highlighted-text); "
        "selection-background-color: palette(highlight); }");
    m_reportSelector->addItems({
        "1. Сводный отчет по заказам и итоговой стоимости (Клиент + Услуга + Деталь)",
        "2. Все гитары клиентов (поиск по фамилии клиента)",
        "3. Прайс-лист услуг с привязкой к мастерам",
        "4. Выручка по каждому мастеру"
    });

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setMinimumWidth(240);
    m_filterEdit->setPlaceholderText(
        "Фильтр по фамилии клиента (например: Иванов)");

    auto *runButton = new QPushButton("Сформировать отчет", this);
    QPixmap playPixmap(18, 18);
    playPixmap.fill(Qt::transparent);
    {
        QPainter painter(&playPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#188038"));
        painter.drawPolygon(
            QPolygonF{QPointF(5, 3), QPointF(14, 9), QPointF(5, 15)});
    }
    runButton->setIcon(QIcon(playPixmap));
    runButton->setIconSize(QSize(18, 18));
    connect(runButton, &QPushButton::clicked, this, &ReportsWidget::runReport);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(m_filterEdit, 1);
    filterLayout->addWidget(runButton);

    m_reportModel = new QSqlQueryModel(this);
    m_reportView = new QTableView(this);
    m_reportView->setModel(m_reportModel);
    m_reportView->setWordWrap(true);
    m_reportView->setTextElideMode(Qt::ElideNone);
    m_reportView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    auto *header = m_reportView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setMinimumSectionSize(96);
    header->setMaximumSectionSize(240);
    header->setResizeContentsPrecision(100);
    m_reportView->verticalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);

    layout->addWidget(m_reportSelector);
    layout->addLayout(filterLayout);
    layout->addWidget(m_reportView);
    runReport();
}

void ReportsWidget::runReport() {
    const int reportIndex = m_reportSelector->currentIndex();
    const QString filter = m_filterEdit->text().trimmed();

    if (reportIndex == 0) {
        m_reportModel->setQuery(R"sql(
            SELECT c.fullname AS "Клиент",
                   s.name AS "Услуга", s.cost AS "Цена услуги",
                   comp.name AS "Комплектующее", comp.cost AS "Цена детали",
                   (s.cost + comp.cost) AS "Итого к оплате (₽)"
            FROM component_costs cc
            JOIN clients c ON cc.clients_id = c.id
            JOIN services s ON cc.services_id = s.id
            JOIN components comp ON cc.components_id = comp.id
            ORDER BY c.fullname
        )sql");
    } else if (reportIndex == 1) {
        QSqlQuery query;
        query.prepare(R"sql(
            SELECT c.fullname AS "Владелец", c.mobile_number AS "Телефон",
                   g.type AS "Тип", g.model AS "Модель",
                   g.serial_number AS "Серийный номер"
            FROM guitars g
            JOIN clients c ON g.client_id = c.id
            WHERE c.fullname ILIKE :f
        )sql");
        query.bindValue(":f", "%" + filter + "%");
        if (!query.exec()) {
            m_reportModel->setQuery(QSqlQuery());
            QMessageBox::warning(this, "Ошибка запроса",
                                 query.lastError().text());
            return;
        }
        m_reportModel->setQuery(std::move(query));
    } else if (reportIndex == 2) {
        m_reportModel->setQuery(R"sql(
            SELECT s.name AS "Услуга", s.cost AS "Стоимость (₽)",
                   m.fullname AS "Мастер", m.specialization AS "Специализация"
            FROM services s
            LEFT JOIN masters m ON s.master_id = m.id
        )sql");
    } else if (reportIndex == 3) {
        m_reportModel->setQuery(R"sql(
            SELECT m.fullname AS "Мастер",
                   COUNT(cc.services_id) AS "Выполнено заказов",
                   COALESCE(SUM(s.cost), 0) AS "Сумма за работы (₽)"
            FROM masters m
            LEFT JOIN services s ON s.master_id = m.id
            LEFT JOIN component_costs cc ON cc.services_id = s.id
            GROUP BY m.fullname
        )sql");
    }

    m_reportView->resizeColumnsToContents();
    m_reportView->resizeRowsToContents();

    if (m_reportModel->lastError().isValid()) {
        QMessageBox::warning(this, "Ошибка запроса",
                             m_reportModel->lastError().text());
    }
}
