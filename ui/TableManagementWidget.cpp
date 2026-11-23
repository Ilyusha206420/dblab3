#include "TableManagementWidget.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QSqlError>
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>
#include <QTabWidget>
#include <QTableView>
#include <QVBoxLayout>
#include <QStringList>

TableManagementWidget::TableManagementWidget(const QString &role,
                                             QWidget *parent)
    : QWidget(parent), m_role(role) {
    auto *layout = new QVBoxLayout(this);
    m_tableTabs = new QTabWidget(this);
    setupTables();

    auto *buttonLayout = new QHBoxLayout;
    m_addButton = new QPushButton("Добавить запись", this);
    m_deleteButton = new QPushButton("Удалить выбранную", this);
    m_saveButton = new QPushButton("Сохранить изменения в БД", this);
    m_revertButton = new QPushButton("Отменить правки", this);
    m_addButton->setIcon(
        m_addButton->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_deleteButton->setIcon(
        m_deleteButton->style()->standardIcon(QStyle::SP_TrashIcon));
    m_saveButton->setIcon(
        m_saveButton->style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_revertButton->setIcon(
        m_revertButton->style()->standardIcon(QStyle::SP_DialogResetButton));

    connect(m_addButton, &QPushButton::clicked, this,
            &TableManagementWidget::addRow);
    connect(m_deleteButton, &QPushButton::clicked, this,
            &TableManagementWidget::deleteRow);
    connect(m_saveButton, &QPushButton::clicked, this,
            &TableManagementWidget::saveChanges);
    connect(m_revertButton, &QPushButton::clicked, this,
            &TableManagementWidget::revertChanges);
    connect(m_tableTabs, &QTabWidget::currentChanged, this,
            &TableManagementWidget::updatePermissions);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_revertButton);
    buttonLayout->addWidget(m_saveButton);

    layout->addWidget(m_tableTabs);
    layout->addLayout(buttonLayout);
    updatePermissions();
}

void TableManagementWidget::setupTables() {
    struct TableSpec {
        QString databaseName;
        QString title;
        QMap<int, QSqlRelation> relations;
        QStringList headers;
    };

    QList<TableSpec> tableSpecs = {
        {"clients", "Клиенты", {}, {"ID", "ФИО клиента", "Телефон", "Email"}},
        {"guitars", "Гитары", {{1, QSqlRelation("clients", "id", "fullname")}},
         {"ID", "Владелец (Клиент)", "Тип гитары", "Модель", "Серийный номер"}},
        {"masters", "Мастера", {}, {"ID", "ФИО мастера", "Специализация"}},
        {"services", "Услуги", {{1, QSqlRelation("masters", "id", "fullname")}},
         {"ID", "Мастер", "Название услуги", "Стоимость (₽)"}},
        {"components", "Комплектующие", {}, {"ID", "Наименование", "Цена (₽)"}},
        {"component_costs", "Заказы (Услуги + Детали)",
         {
             {0, QSqlRelation("components", "id", "name")},
             {1, QSqlRelation("clients", "id", "fullname")},
             {2, QSqlRelation("services", "id", "name")}
         },
         {"Комплектующее", "Клиент", "Услуга"}}
    };

    if (m_role == "admin") {
        tableSpecs.append(
            {"app_users", "Пользователи системы", {},
             {"ID", "Логин", "Пароль", "Роль"}});
    }

    for (const TableSpec &spec : tableSpecs) {
        auto *model = new QSqlRelationalTableModel(this);
        model->setTable(spec.databaseName);
        model->setEditStrategy(QSqlTableModel::OnManualSubmit);

        for (auto relation = spec.relations.cbegin();
             relation != spec.relations.cend(); ++relation) {
            model->setRelation(relation.key(), relation.value());
        }

        for (int column = 0; column < spec.headers.size(); ++column) {
            model->setHeaderData(column, Qt::Horizontal, spec.headers[column]);
        }

        model->select();

        auto *view = new QTableView(this);
        view->setModel(model);
        view->setItemDelegate(new QSqlRelationalDelegate(view));
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setWordWrap(true);
        view->setTextElideMode(Qt::ElideNone);
        view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

        auto *header = view->horizontalHeader();
        header->setSectionResizeMode(QHeaderView::Stretch);
        header->setMinimumSectionSize(90);
        header->setMaximumSectionSize(480);
        view->verticalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);

        if (spec.databaseName != "component_costs") {
            view->hideColumn(0);
        }

        m_models.append(model);
        m_views.append(view);
        m_tableTabs->addTab(view, spec.title);
    }
}

void TableManagementWidget::updatePermissions() {
    QSqlRelationalTableModel *model = currentModel();
    QTableView *view = currentView();
    if (!model || !view) {
        return;
    }

    view->resizeRowsToContents();

    const QString table = model->tableName();

    if (m_role == "admin") {
        view->setEditTriggers(
            QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        m_addButton->setEnabled(true);
        m_deleteButton->setEnabled(true);
        m_saveButton->setEnabled(true);
    } else if (m_role == "master") {
        const bool canEdit = table == "guitars" || table == "component_costs";
        view->setEditTriggers(
            canEdit ? QAbstractItemView::DoubleClicked
                    : QAbstractItemView::NoEditTriggers);
        m_addButton->setEnabled(canEdit);
        m_deleteButton->setEnabled(table == "component_costs");
        m_saveButton->setEnabled(canEdit);
    } else {
        const bool canAdd = table == "clients" || table == "guitars";
        view->setEditTriggers(
            canAdd ? QAbstractItemView::DoubleClicked
                   : QAbstractItemView::NoEditTriggers);
        m_addButton->setEnabled(canAdd);
        m_deleteButton->setEnabled(false);
        m_saveButton->setEnabled(canAdd);
    }
}

void TableManagementWidget::refreshRowHeights() {
    if (QTableView *view = currentView()) {
        view->resizeRowsToContents();
    }
}

QSqlRelationalTableModel *TableManagementWidget::currentModel() const {
    const int index = m_tableTabs->currentIndex();
    return index >= 0 && index < m_models.size() ? m_models[index] : nullptr;
}

QTableView *TableManagementWidget::currentView() const {
    const int index = m_tableTabs->currentIndex();
    return index >= 0 && index < m_views.size() ? m_views[index] : nullptr;
}

void TableManagementWidget::addRow() {
    QSqlRelationalTableModel *model = currentModel();
    if (!model) {
        return;
    }

    const int row = model->rowCount();
    model->insertRow(row);
    currentView()->selectRow(row);
    emit statusMessageChanged(
        "Добавлена пустая строка. Заполните поля и нажмите "
        "«Сохранить изменения в БД».",
        0);
}

void TableManagementWidget::deleteRow() {
    QSqlRelationalTableModel *model = currentModel();
    QTableView *view = currentView();
    if (!model || !view) {
        return;
    }

    const QModelIndex currentIndex = view->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::information(this, "Внимание", "Выберите строку для удаления.");
        return;
    }

    model->removeRow(currentIndex.row());
    emit statusMessageChanged(
        "Строка помечена на удаление. Нажмите «Сохранить изменения в БД» "
        "для подтверждения.",
        0);
}

void TableManagementWidget::saveChanges() {
    QSqlRelationalTableModel *model = currentModel();
    if (!model) {
        return;
    }

    if (!model->submitAll()) {
        QMessageBox::critical(this, "Ошибка сохранения",
                              "Не удалось сохранить данные:\n" +
                                  model->lastError().text());
        return;
    }

    model->select();
    emit statusMessageChanged("Изменения успешно сохранены в PostgreSQL!", 4000);
}

void TableManagementWidget::revertChanges() {
    QSqlRelationalTableModel *model = currentModel();
    if (!model) {
        return;
    }

    model->revertAll();
    emit statusMessageChanged("Несохраненные изменения отменены.", 3000);
}
