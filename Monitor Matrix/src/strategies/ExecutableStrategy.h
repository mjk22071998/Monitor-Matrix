#pragma once
#include "LaunchStrategy.h"

class ExecutableStrategy : public LaunchStrategy {
private:
	QString m_path;
public:
	ExecutableStrategy(const QString& path);

	// Inherited via LaunchStrategy
	virtual void launch(MonitorInfo& monitor, AppPosition position, const QRect& zoneRect = QRect());
};