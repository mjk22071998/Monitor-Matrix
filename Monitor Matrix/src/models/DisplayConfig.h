#pragma once

#include <QString>
#include <QRect>
#include <QSize>
#include <QVector>

struct DisplayResolutionOption {
    QSize size;
    int refreshRate = 60;
};

struct DisplayConfigInfo {
    QString deviceName;
    QString displayName;
    QRect geometry;
    bool isPrimary = false;

    QVector<DisplayResolutionOption> availableResolutions;
};