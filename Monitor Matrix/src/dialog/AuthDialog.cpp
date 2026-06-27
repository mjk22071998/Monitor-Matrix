#include "AuthDialog.h"

#include <qdialogbuttonbox.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qdebug.h>

namespace {

constexpr auto kDefaultUsername = "admin";
constexpr auto kDefaultPassword = "MyP@$$#!";

} // namespace

AuthDialog::AuthDialog(const QString& actionName, QWidget* parent)
    : QDialog(parent),
      m_actionName(actionName)
{
    setWindowTitle(QStringLiteral("Authorization Required"));
    setModal(true);

    auto* layout = new QVBoxLayout(this);

    auto* message = new QLabel(
        QStringLiteral("Enter credentials to continue with %1.")
            .arg(actionName),
        this
    );
    message->setWordWrap(true);
    layout->addWidget(message);

    auto* form = new QFormLayout();

    p_usernameEdit = new QLineEdit(this);
    p_usernameEdit->setText(QString::fromLatin1(kDefaultUsername));
    p_usernameEdit->selectAll();

    p_passwordEdit = new QLineEdit(this);
    p_passwordEdit->setEchoMode(QLineEdit::Password);

    form->addRow(QStringLiteral("Username"), p_usernameEdit);
    form->addRow(QStringLiteral("Password"), p_passwordEdit);
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Unlock"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Close"));

    connect(buttons, &QDialogButtonBox::accepted, this, &AuthDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &AuthDialog::reject);

    layout->addWidget(buttons);

    p_passwordEdit->setFocus();
}

bool AuthDialog::authenticate(QWidget* parent, const QString& actionName)
{
    AuthDialog dialog(actionName, parent);
    return dialog.exec() == QDialog::Accepted;
}

void AuthDialog::onAccepted()
{
    const QString username = p_usernameEdit->text().trimmed();
    const QString password = p_passwordEdit->text();

    if (username == QString::fromLatin1(kDefaultUsername) &&
        password == QString::fromLatin1(kDefaultPassword)) {
        qInfo().noquote() << "Authorization granted for" << m_actionName;
        accept();
        return;
    }

    qWarning().noquote()
        << "Authorization failed for"
        << m_actionName
        << "username="
        << username;

    QMessageBox::warning(
        this,
        QStringLiteral("Authorization Failed"),
        QStringLiteral("The username or password is incorrect.")
    );

    p_passwordEdit->clear();
    p_passwordEdit->setFocus();
}
