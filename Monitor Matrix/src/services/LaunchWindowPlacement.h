#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <QRect>

#include "../enums/AppPosition.h"
#include "../models/MonitorInfo.h"
#include "../platform/windows/WindowsDisplayTopology.h"

namespace LaunchWindowPlacement {

QRect mapZoneToNativeWorkArea(const WindowsNativeMonitor& native,
    const QRect& zoneRect);

bool placeWindow(HWND hwnd,
    const MonitorInfo& info,
    AppPosition pos,
    const QRect& zoneRect);

}
