#pragma once

#include <qdialog.h>

class QLineEdit;

class AuthDialog : public QDialog {
    Q_OBJECT

private:
    QLineEdit* p_usernameEdit = nullptr;
    QLineEdit* p_passwordEdit = nullptr;
    QString m_actionName;

    void onAccepted();

public:
    explicit AuthDialog(const QString& actionName, QWidget* parent = nullptr);

    static bool authenticate(QWidget* parent, const QString& actionName);
};
