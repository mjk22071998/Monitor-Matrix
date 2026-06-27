#include "DisplayConfigService.h"

#include "../platform/windows/WindowsDisplayTopology.h"

DisplayConfigService::DisplayConfigService(QObject* parent)
    : QObject(parent)
{
}

QVector<DisplayConfigInfo> DisplayConfigService::getDisplays() const
{
    QString errorMessage;
    QVector<DisplayConfigInfo> displays =
        WindowsDisplayTopology::getDisplayConfigs(&errorMessage);

    if (displays.isEmpty() && !errorMessage.isEmpty()) {
        auto* self = const_cast<DisplayConfigService*>(this);
        emit self->applyFailed(errorMessage);
    }

    return displays;
}

bool DisplayConfigService::applyDisplays(const QVector<DisplayConfigInfo>& displays)
{
    QString errorMessage;

    if (!WindowsDisplayTopology::applyDisplayConfigs(displays, &errorMessage)) {
        emit applyFailed(errorMessage.isEmpty()
            ? "Could not apply display settings to Windows."
            : errorMessage);
        return false;
    }

    emit displaysApplied();
    return true;
}
