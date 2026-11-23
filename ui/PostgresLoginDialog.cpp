#include "PostgresLoginDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

PostgresLoginDialog::PostgresLoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Подключение к PostgreSQL");
    setMinimumWidth(380);

    auto *layout = new QVBoxLayout(this);
    auto *form = new QFormLayout;

    m_usernameEdit = new QLineEdit("postgres", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    form->addRow("Пользователь PostgreSQL:", m_usernameEdit);
    form->addRow("Пароль PostgreSQL:", m_passwordEdit);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: red;");

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted,
            this, &PostgresLoginDialog::acceptCredentials);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addLayout(form);
    layout->addWidget(m_statusLabel);
    layout->addWidget(buttons);
}

QString PostgresLoginDialog::username() const {
    return m_usernameEdit->text().trimmed();
}

void PostgresLoginDialog::setUsername(const QString &username) {
    if (!username.isEmpty()) {
        m_usernameEdit->setText(username);
    }
}

QString PostgresLoginDialog::password() const {
    return m_passwordEdit->text();
}

void PostgresLoginDialog::acceptCredentials() {
    if (username().isEmpty() || password().isEmpty()) {
        m_statusLabel->setText("Введите пользователя и пароль PostgreSQL.");
        return;
    }

    accept();
}
