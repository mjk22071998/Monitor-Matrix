#pragma once

#include <QDialog>
#include <QVector>
#include "../models/DisplayConfig.h"

class QLabel;
class QComboBox;
class QCheckBox;
class QPushButton;
class QGraphicsView;
class MonitorTopologyScene;
class DisplayConfigService;

class ScreenConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScreenConfigDialog(QWidget* parent = nullptr);

signals:
    void displaySettingsSaved();

private slots:
    void onDisplaySelected(int displayIndex);
    void onResolutionChanged(int index);
    void onPrimaryChanged(bool checked);
    void onSaveClicked();
    void onLayoutChanged();

private:
    DisplayConfigService* m_service = nullptr;
    MonitorTopologyScene* m_scene = nullptr;
    QGraphicsView* m_view = nullptr;

    QLabel* m_selectedDisplayLabel = nullptr;
    QComboBox* m_resolutionCombo = nullptr;
    QCheckBox* m_primaryCheck = nullptr;
    QPushButton* m_saveButton = nullptr;
    QPushButton* m_closeButton = nullptr;

    QVector<DisplayConfigInfo> m_displays;
    QVector<DisplayConfigInfo> m_originalDisplays;

    int m_selectedIndex = -1;
    bool m_hasChanges = false;
    bool m_updatingUi = false;
    QString m_lastApplyError;

    void buildUi();
    void loadDisplays();
    void updateSettingsPanel();
    void markChanged();
    void updateSaveButton();
    bool hasDisplayChanges() const;
};
