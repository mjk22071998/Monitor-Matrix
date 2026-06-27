#pragma once
#include "../enums/AppPosition.h"
#include "../services/MonitorService.h"
#include "../services/LaunchService.h"
#include <qstring.h>
#include <qrect.h>

class LaunchStrategy {
protected:
	LaunchService* p_launchService = nullptr;
	bool ensureLaunchService();
public:
	LaunchStrategy();
	virtual void launch(MonitorInfo& monitor, AppPosition position, const QRect& zoneRect = QRect()) = 0;
	virtual ~LaunchStrategy() = default;
};
