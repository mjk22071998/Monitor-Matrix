#pragma once

#include <qdialog.h>

class QLineEdit;
class QPushButton;
class QComboBox;

class AppEditDialog : public QDialog {
    Q_OBJECT

private:
    QComboBox* p_typeCombo = nullptr;
    QLineEdit* p_nameEdit = nullptr;
    QLineEdit* p_targetEdit = nullptr;
    QPushButton* p_browseButton = nullptr;
    QPushButton* p_addButton = nullptr;

    void browseTarget();
    void updateMode();
    void updateAddButton();
    void applyTargetDefaults(const QString& target);

public:
    explicit AppEditDialog(QWidget* parent = nullptr);
    explicit AppEditDialog(const QString& target, QWidget* parent = nullptr);

    QString appType() const;
    QString appName() const;
    QString target() const;
};
