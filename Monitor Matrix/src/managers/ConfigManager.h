#pragma once
#include<qlist.h>
#include "../models/AppItem.h"

class ConfigManager {
public:
	static QList<AppItem> getApplications(const QString& fileName);
	static bool saveApplications(const QString& fileName, const QList<AppItem>& apps);
	static AppItem createApplicationFromExecutable(
		const QString& executablePath,
		const QString& displayName,
		const QList<AppItem>& existingApps
	);
	static AppItem createUrlLink(
		const QString& url,
		const QString& displayName,
		const QList<AppItem>& existingApps
	);
};
