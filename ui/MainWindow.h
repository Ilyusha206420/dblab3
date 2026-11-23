#pragma once

#include <QMainWindow>

class QString;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, const QString &role,
                        QWidget *parent = nullptr);

private slots:
    void logout();
};
