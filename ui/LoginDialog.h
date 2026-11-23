#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString getUsername() const;
    QString getRole() const;

private slots:
    void onLoginClicked();

private:
    QLineEdit *m_userEdit = nullptr;
    QLineEdit *m_passEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
    QString m_username;
    QString m_role;
};
