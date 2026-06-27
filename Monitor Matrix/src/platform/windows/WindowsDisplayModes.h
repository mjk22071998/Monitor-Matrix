#pragma once

#include <QString>
#include <QVector>

#include "../../models/DisplayConfig.h"

namespace WindowsDisplayModes {

void addWindowsProvidedResolutions(
    QVector<DisplayResolutionOption>& resolutions,
    const QString& deviceName);

bool applyDisplaySettingsApi(
    const QVector<DisplayConfigInfo>& displays,
    QString* errorMessage);

}
