#include <QApplication>
#include <QColor>
#include <QDialog>
#include <QMessageBox>
#include <QPalette>
#include <QSqlDatabase>
#include <QStyle>

#include "database/DatabaseCredentialsStore.h"
#include "database/DatabaseInitializer.h"
#include "ui/LoginDialog.h"
#include "ui/MainWindow.h"
#include "ui/PostgresLoginDialog.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("GuitarWorkshopApp");

    QPalette palette = app.style()->standardPalette();
    palette.setColor(QPalette::Window, QColor("#f4f6f8"));
    palette.setColor(QPalette::WindowText, QColor("#202124"));
    palette.setColor(QPalette::Base, QColor("#ffffff"));
    palette.setColor(QPalette::AlternateBase, QColor("#f7f8fa"));
    palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
    palette.setColor(QPalette::ToolTipText, QColor("#202124"));
    palette.setColor(QPalette::Text, QColor("#202124"));
    palette.setColor(QPalette::Button, QColor("#f1f3f4"));
    palette.setColor(QPalette::ButtonText, QColor("#202124"));
    palette.setColor(QPalette::BrightText, QColor("#c5221f"));
    palette.setColor(QPalette::Link, QColor("#1a73e8"));
    palette.setColor(QPalette::Highlight, QColor("#d2e3fc"));
    palette.setColor(QPalette::HighlightedText, QColor("#174ea6"));
    palette.setColor(QPalette::Light, QColor("#ffffff"));
    palette.setColor(QPalette::Midlight, QColor("#e8eaed"));
    palette.setColor(QPalette::Mid, QColor("#dadce0"));
    palette.setColor(QPalette::Dark, QColor("#bdc1c6"));
    palette.setColor(QPalette::Shadow, QColor("#80868b"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#80868b"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                     QColor("#80868b"));
    app.setPalette(palette);

    if (!QSqlDatabase::isDriverAvailable("QPSQL")) {
        QMessageBox::critical(nullptr, "Ошибка драйвера",
                              "Драйвер QPSQL не найден. Убедитесь, что плагин "
                              "PostgreSQL для Qt и его зависимости находятся рядом "
                              "с приложением.");
        return 1;
    }

    DatabaseInitializer database;
    DatabaseCredentialsStore credentialsStore;
    DatabaseCredentials credentials;
    QString error;
    const DatabaseCredentialsStore::ReadResult readResult =
        credentialsStore.read(credentials, error);
    if (readResult == DatabaseCredentialsStore::ReadResult::Failed) {
        QMessageBox::warning(
            nullptr, "Данные подключения",
            "Не удалось прочитать сохраненные данные PostgreSQL:\n" + error +
                "\nВведите данные подключения вручную.");
    }

    bool connected = false;
    if (readResult == DatabaseCredentialsStore::ReadResult::Found) {
        bool credentialsRejected = false;
        if (database.connectToPostgres(credentials.username,
                                       credentials.password, error,
                                       credentialsRejected)) {
            connected = true;
        } else if (!credentialsRejected) {
            QMessageBox::critical(nullptr, "Ошибка PostgreSQL", error);
            return 1;
        }
    }

    while (!connected) {
        PostgresLoginDialog postgresLogin;
        postgresLogin.setUsername(credentials.username);
        if (postgresLogin.exec() != QDialog::Accepted) {
            return 0;
        }

        credentials.username = postgresLogin.username();
        credentials.password = postgresLogin.password();

        bool credentialsRejected = false;
        if (!database.connectToPostgres(credentials.username,
                                        credentials.password, error,
                                        credentialsRejected)) {
            if (credentialsRejected) {
                QMessageBox::warning(nullptr, "Ошибка PostgreSQL", error);
                continue;
            }

            QMessageBox::critical(nullptr, "Ошибка PostgreSQL", error);
            return 1;
        }

        connected = true;
        if (!credentialsStore.write(credentials, error)) {
            QMessageBox::warning(
                nullptr, "Кэширование подключения",
                "Подключение установлено, но не удалось сохранить данные "
                "в текстовый файл:\n" +
                    error);
        }
    }

    if (!database.initialize(error)) {
        QMessageBox::critical(nullptr, "Ошибка инициализации БД", error);
        return 1;
    }

    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window(login.getUsername(), login.getRole());
    window.show();

    return app.exec();
}