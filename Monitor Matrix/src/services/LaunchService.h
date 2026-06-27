#pragma once
#include <qstring.h>
#include <qrect.h>
#include "../models/MonitorInfo.h"
#include "../enums/AppPosition.h"

class LaunchService{
private:
	explicit LaunchService();
	static QString getChromePath();

public:
	static LaunchService* getInstance();

	bool launchExecutable(QString path, MonitorInfo info, AppPosition pos);
	bool launchExecutable(QString path, MonitorInfo info, AppPosition pos, const QRect& zoneRect);

	bool launchURL(QString url, MonitorInfo info, AppPosition pos);
	bool launchURL(QString url, MonitorInfo info, AppPosition pos, const QRect& zoneRect);
};
