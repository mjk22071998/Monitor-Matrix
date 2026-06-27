#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <QRect>

#include "../enums/AppPosition.h"
#include "../models/MonitorInfo.h"

namespace LaunchWindowPlacement {

bool placeWindow(HWND hwnd,
    const MonitorInfo& info,
    AppPosition pos,
    const QRect& zoneRect);

}
