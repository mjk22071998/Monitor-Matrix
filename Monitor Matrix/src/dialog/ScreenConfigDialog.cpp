#include "ScreenConfigDialog.h"

#include "../scene/MonitorTopologyScene.h"
#include "../services/DisplayConfigService.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ScreenConfigDialog::ScreenConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    m_service = new DisplayConfigService(this);

    connect(m_service, &DisplayConfigService::applyFailed,
        this, [this](const QString& message) {
            m_lastApplyError = message;
        });

    buildUi();
    loadDisplays();

    setWindowTitle("Screen Configuration");
    resize(900, 650);
}

void ScreenConfigDialog::buildUi()
{
    auto* mainLayout = new QVBoxLayout(this);

    m_scene = new MonitorTopologyScene(this);

    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing, true);
    m_view->setMinimumHeight(360);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
    mainLayout->addWidget(m_view, 1);

    auto* settingsGroup = new QGroupBox("Display Settings", this);
    auto* settingsLayout = new QGridLayout(settingsGroup);

    m_selectedDisplayLabel = new QLabel("No screen selected", settingsGroup);

    m_resolutionCombo = new QComboBox(settingsGroup);
    m_primaryCheck = new QCheckBox("Make this my primary display", settingsGroup);

    settingsLayout->addWidget(new QLabel("Selected screen:", settingsGroup), 0, 0);
    settingsLayout->addWidget(m_selectedDisplayLabel, 0, 1);

    settingsLayout->addWidget(new QLabel("Resolution:", settingsGroup), 1, 0);
    settingsLayout->addWidget(m_resolutionCombo, 1, 1);

    settingsLayout->addWidget(m_primaryCheck, 2, 0, 1, 2);

    mainLayout->addWidget(settingsGroup);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_saveButton = new QPushButton("Save", this);
    m_saveButton->setObjectName("runButton");
    m_saveButton->setEnabled(false);
    buttonLayout->addWidget(m_saveButton);

    m_closeButton = new QPushButton("Close", this);
    m_closeButton->setObjectName("secondaryButton");
    buttonLayout->addWidget(m_closeButton);

    mainLayout->addLayout(buttonLayout);

    connect(m_scene, &MonitorTopologyScene::displaySelected,
        this, &ScreenConfigDialog::onDisplaySelected);

    connect(m_scene, &MonitorTopologyScene::layoutChanged,
        this, &ScreenConfigDialog::onLayoutChanged);

    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ScreenConfigDialog::onResolutionChanged);

    connect(m_primaryCheck, &QCheckBox::toggled,
        this, &ScreenConfigDialog::onPrimaryChanged);

    connect(m_closeButton, &QPushButton::clicked,
        this, &ScreenConfigDialog::reject);

    connect(m_saveButton, &QPushButton::clicked,
        this, &ScreenConfigDialog::onSaveClicked);
}

void ScreenConfigDialog::loadDisplays()
{
    m_displays = m_service->getDisplays();
    m_originalDisplays = m_displays;

    m_scene->setDisplays(m_displays);

    if (!m_displays.isEmpty())
        onDisplaySelected(0);

    updateSaveButton();
}

bool ScreenConfigDialog::hasDisplayChanges() const
{
    if (m_displays.size() != m_originalDisplays.size())
        return true;

    for (int i = 0; i < m_displays.size(); ++i) {
        const DisplayConfigInfo& a = m_displays[i];
        const DisplayConfigInfo& b = m_originalDisplays[i];

        if (a.deviceName != b.deviceName)
            return true;

        if (a.geometry != b.geometry)
            return true;

        if (a.isPrimary != b.isPrimary)
            return true;
    }

    return false;
}

void ScreenConfigDialog::onDisplaySelected(int displayIndex)
{
    if (displayIndex < 0 || displayIndex >= m_displays.size())
        return;

    m_selectedIndex = displayIndex;
    updateSettingsPanel();
}

void ScreenConfigDialog::updateSettingsPanel()
{
    if (m_selectedIndex < 0 || m_selectedIndex >= m_displays.size())
        return;

    m_updatingUi = true;

    const DisplayConfigInfo& display = m_displays[m_selectedIndex];

    m_selectedDisplayLabel->setText(
        QString("%1 (%2)").arg(display.displayName, display.deviceName)
    );

    m_resolutionCombo->clear();

    const QSize currentSize = display.geometry.size();
    int selectedResolutionIndex = 0;

    for (int i = 0; i < display.availableResolutions.size(); ++i) {
        const auto& option = display.availableResolutions[i];

        QString text = QString("%1 x %2")
            .arg(option.size.width())
            .arg(option.size.height());

        m_resolutionCombo->addItem(text);

        if (option.size == currentSize)
            selectedResolutionIndex = i;
    }

    m_resolutionCombo->setCurrentIndex(selectedResolutionIndex);
    m_primaryCheck->setChecked(display.isPrimary);

    m_updatingUi = false;
}

void ScreenConfigDialog::onResolutionChanged(int index)
{
    if (m_updatingUi)
        return;

    if (m_selectedIndex < 0 || m_selectedIndex >= m_displays.size())
        return;

    if (index < 0 ||
        index >= m_displays[m_selectedIndex].availableResolutions.size()) {
        return;
    }

    const QSize oldSize = m_displays[m_selectedIndex].geometry.size();
    const QSize newSize =
        m_displays[m_selectedIndex].availableResolutions[index].size;

    if (oldSize == newSize)
        return;

    const bool accepted =
        m_scene->updateDisplaySize(m_selectedIndex, newSize);

    if (!accepted) {
        QMessageBox::warning(
            this,
            "Invalid Resolution",
            "This resolution would make the screen layout invalid.\n\n"
            "Screens must not overlap and must remain connected with at least "
            "15px side contact."
        );

        updateSettingsPanel();
        return;
    }

    m_displays = m_scene->displays();

    markChanged();
}

void ScreenConfigDialog::onPrimaryChanged(bool checked)
{
    if (m_updatingUi || !checked)
        return;

    if (m_selectedIndex < 0 || m_selectedIndex >= m_displays.size())
        return;

    for (int i = 0; i < m_displays.size(); ++i)
        m_displays[i].isPrimary = (i == m_selectedIndex);

    m_scene->updatePrimaryDisplay(m_selectedIndex);

    markChanged();
}

void ScreenConfigDialog::onLayoutChanged()
{
    m_displays = m_scene->displays();
    markChanged();
}

void ScreenConfigDialog::markChanged()
{
    m_hasChanges = true;
    updateSaveButton();
}

void ScreenConfigDialog::updateSaveButton()
{
    m_displays = m_scene->displays();
    m_hasChanges = hasDisplayChanges();

    m_saveButton->setEnabled(m_hasChanges);
}

void ScreenConfigDialog::onSaveClicked()
{
    m_displays = m_scene->displays();
    m_hasChanges = hasDisplayChanges();

    if (!m_hasChanges) {
        return;
    }

    m_lastApplyError.clear();

    if (!m_service->applyDisplays(m_displays)) {
        QMessageBox::warning(
            this,
            "Display Settings",
            m_lastApplyError.isEmpty()
            ? "Could not apply display settings to Windows."
            : m_lastApplyError
        );
        return;
    }

    m_originalDisplays = m_displays;
    m_hasChanges = false;
    updateSaveButton();

    emit displaySettingsSaved();
}
