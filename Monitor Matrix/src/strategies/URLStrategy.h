#pragma once
#include "LaunchStrategy.h"

class URLStrategy : public LaunchStrategy {
private:
	QString m_url;
public:
	URLStrategy(const QString& path);

	// Inherited via LaunchStrategy
	virtual void launch(MonitorInfo& monitor, AppPosition position, const QRect& zoneRect = QRect());
};