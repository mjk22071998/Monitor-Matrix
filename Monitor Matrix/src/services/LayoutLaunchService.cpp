#include "LayoutLaunchService.h"

#include <algorithm>

#include <QDebug>

#include "../platform/windows/WindowsTaskRunner.h"
#include "../strategies/LaunchStrategy.h"

void LayoutLaunchService::launchPlacements(
	const QList<AppItem>& apps,
	const QList<Placement>& placements) const
{
	qInfo() << "Run clicked. placementCount=" << placements.size();

	for (const Placement& placement : placements) {
		auto it = std::find_if(apps.begin(), apps.end(), [&](const AppItem& app) {
			return app.id == placement.appId;
		});

		if (it == apps.end() || !it->launchStrategy) {
			continue;
		}

		qInfo().noquote()
			<< "Launching placement"
			<< "placementId=" << placement.placementId
			<< "appId=" << placement.appId
			<< "monitor=" << placement.monitor.deviceName
			<< "custom=" << placement.customRect
			<< "zone=" << QString("x=%1 y=%2 w=%3 h=%4")
				.arg(placement.zoneRect.x())
				.arg(placement.zoneRect.y())
				.arg(placement.zoneRect.width())
				.arg(placement.zoneRect.height());

		auto launchStrategy = it->launchStrategy;
		MonitorInfo monitor = placement.monitor;
		AppPosition position = placement.position;

		if (placement.customRect) {
			QRect zone = placement.zoneRect;
			WindowsTaskRunner::run([launchStrategy, monitor, position, zone]() mutable {
				launchStrategy->launch(monitor, position, zone);
			});
		}
		else {
			WindowsTaskRunner::run([launchStrategy, monitor, position]() mutable {
				launchStrategy->launch(monitor, position);
			});
		}
	}
}
