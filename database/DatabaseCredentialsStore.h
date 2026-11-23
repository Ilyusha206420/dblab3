#pragma once

#include <QString>

struct DatabaseCredentials {
    QString username;
    QString password;
};

class DatabaseCredentialsStore final {
public:
    enum class ReadResult {
        Found,
        NotFound,
        Failed
    };

    ReadResult read(DatabaseCredentials &credentials, QString &error);
    bool write(const DatabaseCredentials &credentials, QString &error);
};
