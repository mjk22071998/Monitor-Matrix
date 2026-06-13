#pragma once
#include <qstring.h>
#include "../services/MonitorService.h"
#include "../enums/AppPositionEnum.h"
#include <quuid.h>

struct Placement {
	QString placementId;
	QString appId;
	MonitorInfo monitor;
	AppPosition position = AppPosition::FULL_SCREEN;
	QRect zoneRect;
	bool customRect = false;

	Placement() = default;

	Placement(const QString& appId_, const MonitorInfo& monitor_, AppPosition position_)
		: placementId(QUuid::createUuid().toString(QUuid::WithoutBraces)),
		appId(appId_),
		monitor(monitor_),
		position(position_) {
	}

	Placement(const QString& appId_, const MonitorInfo& monitor_, const QRect& zoneRect_)
		: placementId(QUuid::createUuid().toString(QUuid::WithoutBraces)),
		appId(appId_),
		monitor(monitor_),
		position(AppPosition::FULL_SCREEN),
		zoneRect(zoneRect_),
		customRect(true) {
	}
};
