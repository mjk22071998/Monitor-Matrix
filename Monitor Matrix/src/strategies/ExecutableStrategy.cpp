#include "ExecutableStrategy.h"

#include <qdebug.h>

ExecutableStrategy::ExecutableStrategy(const QString& path) {
	m_path = path.trimmed();
}

void ExecutableStrategy::launch(MonitorInfo& monitor, AppPosition position, const QRect& zoneRect)
{
	if (!ensureLaunchService()) {
		qWarning() << "Cannot launch executable because LaunchService is unavailable";
		return;
	}

	if (m_path.isEmpty()) {
		qWarning() << "Executable path is empty";
		return;
	}

	if (zoneRect.isNull()) {
		p_launchService->launchExecutable(m_path, monitor, position);
	}
	else {
		p_launchService->launchExecutable(m_path, monitor, position, zoneRect);
	}
}