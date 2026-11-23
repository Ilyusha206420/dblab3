#pragma once

#include <QDialog>
#include <QString>

class QLabel;
class QLineEdit;

class PostgresLoginDialog final : public QDialog {
    Q_OBJECT

public:
    explicit PostgresLoginDialog(QWidget *parent = nullptr);

    void setUsername(const QString &username);
    QString username() const;
    QString password() const;

private slots:
    void acceptCredentials();

private:
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
};
