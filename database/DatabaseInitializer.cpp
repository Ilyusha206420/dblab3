#include "DatabaseInitializer.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

namespace {

const QString kDatabaseName = QStringLiteral("guitar_workshop");
constexpr int kPostgresPort = 5432;

bool isCredentialRejection(const QSqlError &error) {
    const QString code = error.nativeErrorCode();
    if (code == "28P01" || code == "28000") {
        return true;
    }

    const QString message =
        (error.driverText() + " " + error.databaseText()).toLower();
    return message.contains("password authentication failed") ||
           (message.contains("role") && message.contains("does not exist"));
}

bool ensureApplicationDatabase(QSqlDatabase &database, QString &error,
                               bool &credentialsRejected) {
    if (!database.open()) {
        credentialsRejected = isCredentialRejection(database.lastError());
        error = QStringLiteral("Не удалось подключиться к PostgreSQL:\n%1")
                    .arg(database.lastError().text());
        return false;
    }

    bool databaseExists = false;
    {
        QSqlQuery query(database);
        query.prepare(
            "SELECT EXISTS (SELECT 1 FROM pg_database WHERE datname = :name)");
        query.bindValue(":name", kDatabaseName);

        if (!query.exec() || !query.next()) {
            error = QStringLiteral("Не удалось проверить наличие базы данных:\n%1")
                        .arg(query.lastError().text());
            return false;
        }

        databaseExists = query.value(0).toBool();
    }

    if (!databaseExists) {
        const QString quotedName = database.driver()->escapeIdentifier(
            kDatabaseName, QSqlDriver::TableName);
        QSqlQuery query(database);
        if (!query.exec(QStringLiteral("CREATE DATABASE %1").arg(quotedName))) {
            error = QStringLiteral("Не удалось создать базу guitar_workshop:\n%1")
                        .arg(query.lastError().text());
            return false;
        }
    }

    database.close();
    database.setDatabaseName(kDatabaseName);
    if (!database.open()) {
        error = QStringLiteral("Не удалось открыть базу guitar_workshop:\n%1")
                    .arg(database.lastError().text());
        return false;
    }

    return true;
}

bool executeSqlFile(const QSqlDatabase &database, const QString &fileName,
                    QString &error) {
    const QString schemaPath =
        QDir(QCoreApplication::applicationDirPath()).filePath(fileName);
    QFile schemaFile(schemaPath);
    if (!schemaFile.open(QIODevice::ReadOnly)) {
        error = QStringLiteral("Не удалось открыть %1 рядом с приложением.")
                    .arg(fileName);
        return false;
    }

    QSqlQuery query(database);
    if (!query.exec(QString::fromUtf8(schemaFile.readAll()))) {
        error = QStringLiteral("Не удалось выполнить %1:\n%2")
                    .arg(fileName, query.lastError().text());
        return false;
    }

    return true;
}

bool applySchemaIfEmpty(const QSqlDatabase &database, QString &error) {
    bool hasTables = false;
    {
        QSqlQuery query(database);
        const QString tableCheck = QStringLiteral(
            "SELECT EXISTS (SELECT 1 FROM information_schema.tables "
            "WHERE table_schema = 'public')");

        if (!query.exec(tableCheck) || !query.next()) {
            error = QStringLiteral("Не удалось проверить схему базы данных:\n%1")
                        .arg(query.lastError().text());
            return false;
        }

        hasTables = query.value(0).toBool();
    }

    if (hasTables) {
        return true;
    }

    return executeSqlFile(database, "database.sql", error);
}

}

DatabaseInitializer::DatabaseInitializer()
    : m_database(QSqlDatabase::addDatabase("QPSQL")) {}

bool DatabaseInitializer::connectToPostgres(const QString &username,
                                             const QString &password,
                                             QString &error,
                                             bool &credentialsRejected) {
    m_database.setHostName("localhost");
    m_database.setPort(kPostgresPort);
    m_database.setDatabaseName("postgres");
    m_database.setUserName(username);
    m_database.setPassword(password);

    credentialsRejected = false;
    return ensureApplicationDatabase(m_database, error, credentialsRejected);
}

bool DatabaseInitializer::initialize(QString &error) {
    return applySchemaIfEmpty(m_database, error);
}
