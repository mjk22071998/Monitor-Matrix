#include "AppEditDialog.h"

#include <qdialogbuttonbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qformlayout.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qurl.h>

AppEditDialog::AppEditDialog(QWidget* parent)
    : AppEditDialog(QString(), parent)
{
}

AppEditDialog::AppEditDialog(const QString& target, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Add Item"));
    setModal(true);

    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    p_typeCombo = new QComboBox(this);
    p_typeCombo->addItem(QStringLiteral("Application"));
    p_typeCombo->addItem(QStringLiteral("URL"));

    p_nameEdit = new QLineEdit(this);
    p_targetEdit = new QLineEdit(this);

    p_browseButton = new QPushButton(QStringLiteral("Browse"), this);
    connect(p_browseButton, &QPushButton::clicked,
        this, &AppEditDialog::browseTarget);

    auto* pathRow = new QWidget(this);
    auto* pathLayout = new QHBoxLayout(pathRow);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    pathLayout->addWidget(p_targetEdit);
    pathLayout->addWidget(p_browseButton);

    form->addRow(QStringLiteral("Type"), p_typeCombo);
    form->addRow(QStringLiteral("Name"), p_nameEdit);
    form->addRow(QStringLiteral("Target"), pathRow);
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );

    p_addButton = buttons->button(QDialogButtonBox::Ok);
    p_addButton->setText(QStringLiteral("Add"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Close"));

    connect(buttons, &QDialogButtonBox::accepted, this, &AppEditDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &AppEditDialog::reject);
    connect(p_typeCombo, &QComboBox::currentTextChanged,
        this, &AppEditDialog::updateMode);
    connect(p_nameEdit, &QLineEdit::textChanged,
        this, &AppEditDialog::updateAddButton);
    connect(p_targetEdit, &QLineEdit::textChanged,
        this, &AppEditDialog::updateAddButton);

    layout->addWidget(buttons);

    applyTargetDefaults(target);
    updateMode();
    updateAddButton();
}

QString AppEditDialog::appType() const
{
    return p_typeCombo->currentText();
}

QString AppEditDialog::appName() const
{
    return p_nameEdit->text().trimmed();
}

QString AppEditDialog::target() const
{
    return p_targetEdit->text().trimmed();
}

void AppEditDialog::browseTarget()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Choose Application"),
        p_targetEdit->text().isEmpty()
            ? QString()
            : QFileInfo(p_targetEdit->text()).absolutePath(),
        QStringLiteral("Windows Applications and Shortcuts (*.exe *.lnk)")
    );

    if (!fileName.isEmpty()) {
        applyTargetDefaults(fileName);
    }
}

void AppEditDialog::applyTargetDefaults(const QString& target)
{
    if (target.isEmpty()) {
        return;
    }

    const QFileInfo fileInfo(target);
    if (fileInfo.exists() && fileInfo.isFile()) {
        p_typeCombo->setCurrentText(QStringLiteral("Application"));
        p_targetEdit->setText(QDir::toNativeSeparators(fileInfo.absoluteFilePath()));

        if (p_nameEdit->text().trimmed().isEmpty()) {
            p_nameEdit->setText(fileInfo.completeBaseName());
        }

        return;
    }

    p_typeCombo->setCurrentText(QStringLiteral("URL"));
    p_targetEdit->setText(target.trimmed());
}

void AppEditDialog::updateMode()
{
    const bool isUrl = appType() == QStringLiteral("URL");
    p_browseButton->setEnabled(!isUrl);

    if (isUrl) {
        p_targetEdit->setPlaceholderText(QStringLiteral("https://example.com"));
    }
    else {
        p_targetEdit->setPlaceholderText(QStringLiteral("C:\\Path\\Application.exe"));
    }

    updateAddButton();
}

void AppEditDialog::updateAddButton()
{
    bool valid = !appName().isEmpty() && !target().isEmpty();

    if (valid && appType() == QStringLiteral("URL")) {
        const QUrl url = QUrl::fromUserInput(target());
        valid = url.isValid() &&
            !url.scheme().isEmpty() &&
            (url.scheme().compare(QStringLiteral("http"), Qt::CaseInsensitive) == 0 ||
                url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0);
    }
    else if (valid) {
        const QFileInfo fileInfo(target());
        valid =
            fileInfo.exists() &&
            fileInfo.isFile() &&
            (fileInfo.suffix().compare(QStringLiteral("exe"), Qt::CaseInsensitive) == 0 ||
                fileInfo.suffix().compare(QStringLiteral("lnk"), Qt::CaseInsensitive) == 0);
    }

    p_addButton->setEnabled(valid);
}
