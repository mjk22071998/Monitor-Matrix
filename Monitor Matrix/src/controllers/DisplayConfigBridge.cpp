#include "DisplayConfigBridge.h"

#include <QString>

DisplayConfigBridge::DisplayConfigBridge(QObject* parent)
    : QObject(parent)
{
    connect(&m_service, &DisplayConfigService::displaysApplied, this, [this]() {
        m_originalDisplays = m_displays;
        updateChangeState();
        emit applySuccess();
    });

    connect(&m_service, &DisplayConfigService::applyFailed, this, [this](const QString& message) {
        m_lastErrorMessage = message;
        emit applyFailed(message);
    });

    refreshDisplays();
}

void DisplayConfigBridge::setSelectedIndex(int index)
{
    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        emit selectedIndexChanged(m_selectedIndex);
    }
}

void DisplayConfigBridge::refreshDisplays()
{
    m_displays = m_service.getDisplays();
    m_originalDisplays = m_displays;

    if (m_selectedIndex < 0 && !m_displays.isEmpty()) {
        m_selectedIndex = 0;
        emit selectedIndexChanged(m_selectedIndex);
    } else if (m_selectedIndex >= m_displays.size()) {
        m_selectedIndex = m_displays.isEmpty() ? -1 : 0;
        emit selectedIndexChanged(m_selectedIndex);
    }

    updateChangeState();
    emit displaysChanged();
}

QVariantList DisplayConfigBridge::displays() const
{
    QVariantList list;
    list.reserve(m_displays.size());

    for (int i = 0; i < m_displays.size(); ++i) {
        list.append(getDisplay(i));
    }

    return list;
}

QVariantMap DisplayConfigBridge::getDisplay(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_displays.size()) {
        return map;
    }

    const DisplayConfigInfo& info = m_displays.at(index);
    map[QStringLiteral("index")] = index;
    map[QStringLiteral("deviceName")] = info.deviceName;
    map[QStringLiteral("displayName")] = info.displayName;
    map[QStringLiteral("x")] = info.geometry.x();
    map[QStringLiteral("y")] = info.geometry.y();
    map[QStringLiteral("width")] = info.geometry.width();
    map[QStringLiteral("height")] = info.geometry.height();
    map[QStringLiteral("isPrimary")] = info.isPrimary;
    map[QStringLiteral("resolutions")] = getResolutions(index);

    return map;
}

QVariantList DisplayConfigBridge::getResolutions(int displayIndex) const
{
    QVariantList resList;
    if (displayIndex < 0 || displayIndex >= m_displays.size()) {
        return resList;
    }

    const auto& resolutions = m_displays.at(displayIndex).availableResolutions;
    resList.reserve(resolutions.size());

    for (const auto& opt : resolutions) {
        QVariantMap optMap;
        optMap[QStringLiteral("width")] = opt.size.width();
        optMap[QStringLiteral("height")] = opt.size.height();
        optMap[QStringLiteral("refreshRate")] = opt.refreshRate;
        optMap[QStringLiteral("label")] = QStringLiteral("%1 x %2 @ %3Hz")
                                              .arg(opt.size.width())
                                              .arg(opt.size.height())
                                              .arg(opt.refreshRate);
        resList.append(optMap);
    }

    return resList;
}

bool DisplayConfigBridge::setDisplayPosition(int displayIndex, int x, int y)
{
    if (displayIndex < 0 || displayIndex >= m_displays.size()) {
        return false;
    }

    m_displays[displayIndex].geometry.moveTo(x, y);
    updateChangeState();
    emit displaysChanged();
    return true;
}

bool DisplayConfigBridge::setDisplayResolution(int displayIndex, int width, int height)
{
    if (displayIndex < 0 || displayIndex >= m_displays.size()) {
        return false;
    }

    m_displays[displayIndex].geometry.setSize(QSize(width, height));
    updateChangeState();
    emit displaysChanged();
    return true;
}

bool DisplayConfigBridge::setPrimaryDisplay(int displayIndex)
{
    if (displayIndex < 0 || displayIndex >= m_displays.size()) {
        return false;
    }

    for (int i = 0; i < m_displays.size(); ++i) {
        m_displays[i].isPrimary = (i == displayIndex);
    }

    updateChangeState();
    emit displaysChanged();
    return true;
}

bool DisplayConfigBridge::applyChanges()
{
    if (!m_hasChanges) {
        return true;
    }

    m_lastErrorMessage.clear();
    const bool ok = m_service.applyDisplays(m_displays);
    if (ok) {
        m_originalDisplays = m_displays;
        updateChangeState();
        emit applySuccess();
        return true;
    }

    emit applyFailed(m_lastErrorMessage.isEmpty() ? QStringLiteral("Failed to apply display configuration.")
                                                  : m_lastErrorMessage);
    return false;
}

void DisplayConfigBridge::revertChanges()
{
    m_displays = m_originalDisplays;
    updateChangeState();
    emit displaysChanged();
}

bool DisplayConfigBridge::computeHasChanges() const
{
    if (m_displays.size() != m_originalDisplays.size()) {
        return true;
    }

    for (int i = 0; i < m_displays.size(); ++i) {
        const DisplayConfigInfo& a = m_displays.at(i);
        const DisplayConfigInfo& b = m_originalDisplays.at(i);

        if (a.deviceName != b.deviceName ||
            a.geometry != b.geometry ||
            a.isPrimary != b.isPrimary) {
            return true;
        }
    }

    return false;
}

void DisplayConfigBridge::updateChangeState()
{
    const bool changed = computeHasChanges();
    if (m_hasChanges != changed) {
        m_hasChanges = changed;
        emit hasChangesChanged(m_hasChanges);
    }
}
