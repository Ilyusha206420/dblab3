#include "DatabaseCredentialsStore.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace {

const QString kSettingsGroup = QStringLiteral("postgresql");
const QString kSettingsFileName = QStringLiteral("postgres.ini");

QString settingsFilePath() {
    const QString configDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDirectory.isEmpty()) {
        return {};
    }

    return QDir(configDirectory).filePath(kSettingsFileName);
}

}

DatabaseCredentialsStore::ReadResult DatabaseCredentialsStore::read(
    DatabaseCredentials &credentials, QString &error) {
    const QString filePath = settingsFilePath();
    if (filePath.isEmpty()) {
        error = QStringLiteral("Не удалось определить папку настроек приложения.");
        return ReadResult::Failed;
    }

    QSettings settings(filePath, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);
    settings.beginGroup(kSettingsGroup);
    settings.sync();

    if (settings.status() != QSettings::NoError) {
        error = QStringLiteral("Не удалось прочитать файл настроек %1.")
                    .arg(filePath);
        return ReadResult::Failed;
    }

    if (!settings.contains("username") || !settings.contains("password")) {
        return ReadResult::NotFound;
    }

    credentials.username = settings.value("username").toString().trimmed();
    credentials.password = settings.value("password").toString();
    if (credentials.username.isEmpty() || credentials.password.isEmpty()) {
        return ReadResult::NotFound;
    }

    return ReadResult::Found;
}

bool DatabaseCredentialsStore::write(const DatabaseCredentials &credentials,
                                     QString &error) {
    const QString filePath = settingsFilePath();
    if (filePath.isEmpty()) {
        error = QStringLiteral("Не удалось определить папку настроек приложения.");
        return false;
    }

    const QFileInfo fileInfo(filePath);
    if (!QDir().mkpath(fileInfo.absolutePath())) {
        error = QStringLiteral("Не удалось создать папку для файла настроек.");
        return false;
    }

    QSettings settings(filePath, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);
    settings.beginGroup(kSettingsGroup);
    settings.setValue("username", credentials.username);
    settings.setValue("password", credentials.password);
    settings.sync();

    if (settings.status() != QSettings::NoError) {
        error = QStringLiteral("Не удалось записать файл настроек %1.")
                    .arg(filePath);
        return false;
    }

    return true;
}
