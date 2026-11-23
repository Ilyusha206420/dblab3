#include "MainWindow.h"

#include "ReportsWidget.h"
#include "TableManagementWidget.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(const QString &username, const QString &role,
                       QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle(QString("ИС «Гитарная мастерская» — Пользователь: %1 [%2]")
                       .arg(username, role.toUpper()));
    resize(1050, 650);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    auto *topBar = new QHBoxLayout;

    auto *roleLabel =
        new QLabel(QString("<b>Текущая роль:</b> %1").arg(role), this);
    auto *logoutButton = new QPushButton("Сменить пользователя", this);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::logout);

    topBar->addWidget(roleLabel);
    topBar->addStretch();
    topBar->addWidget(logoutButton);
    layout->addLayout(topBar);

    auto *mainTabs = new QTabWidget(this);
    auto *tablesWidget = new TableManagementWidget(role, mainTabs);
    auto *reportsWidget = new ReportsWidget(mainTabs);
    connect(mainTabs, &QTabWidget::currentChanged, this,
            [tablesWidget](int index) {
                if (index == 0) {
                    QTimer::singleShot(
                        0, tablesWidget, &TableManagementWidget::refreshRowHeights);
                }
            });
    connect(tablesWidget, &TableManagementWidget::statusMessageChanged, this,
            [this](const QString &message, int timeout) {
                statusBar()->showMessage(message, timeout);
            });

    mainTabs->addTab(tablesWidget, "Таблицы базы данных");
    mainTabs->addTab(reportsWidget, "Запросы и Отчеты");
    layout->addWidget(mainTabs);

    setCentralWidget(centralWidget);
    statusBar()->showMessage("Подключено к PostgreSQL. Готово к работе.");
    QTimer::singleShot(0, tablesWidget,
                       &TableManagementWidget::refreshRowHeights);
}

void MainWindow::logout() {
    qApp->quit();
    QProcess::startDetached(qApp->arguments().first(), qApp->arguments().mid(1));
}
