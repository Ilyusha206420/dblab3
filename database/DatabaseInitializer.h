#pragma once

#include <QSqlDatabase>
#include <QString>

class DatabaseInitializer final {
public:
    DatabaseInitializer();

    bool connectToPostgres(const QString &username, const QString &password,
                           QString &error, bool &credentialsRejected);
    bool initialize(QString &error);

private:
    QSqlDatabase m_database;
};
