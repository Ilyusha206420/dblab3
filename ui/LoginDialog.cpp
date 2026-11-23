#include "LoginDialog.h"

#include <QFormLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyle>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Авторизация — Гитарная мастерская");
    setMinimumWidth(340);

    auto *mainLayout = new QVBoxLayout(this);
    auto *formLayout = new QFormLayout;

    m_userEdit = new QLineEdit(this);
    m_userEdit->setPlaceholderText("admin / master / manager");

    m_passEdit = new QLineEdit(this);
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPlaceholderText("Введите пароль");

    formLayout->addRow("Логин:", m_userEdit);
    formLayout->addRow("Пароль:", m_passEdit);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: red;");

    auto *loginBtn = new QPushButton("Войти в систему", this);
    loginBtn->setIcon(
        loginBtn->style()->standardIcon(QStyle::SP_DialogApplyButton));
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(loginBtn);
}

QString LoginDialog::getUsername() const {
    return m_username;
}

QString LoginDialog::getRole() const {
    return m_role;
}

void LoginDialog::onLoginClicked() {
    const QString username = m_userEdit->text().trimmed();
    const QString password = m_passEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("Заполните логин и пароль.");
        return;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT role FROM app_users "
        "WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка БД", query.lastError().text());
        return;
    }

    if (query.next()) {
        m_username = username;
        m_role = query.value(0).toString();
        accept();
    } else {
        m_statusLabel->setText("Неверный логин или пароль!");
    }
}
