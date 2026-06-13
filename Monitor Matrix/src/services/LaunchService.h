#pragma once
#include <qstring.h>
#include <qmutex.h>
#include "../models/MonitorInfo.h"
#include "../enums/AppPositionEnum.h"

class LaunchService{
private:
	static LaunchService* p_instance;
	explicit LaunchService();
	static QString getChromePath();

	QMutex m_mutex;

public:
	static LaunchService* getInstance();

	bool launchExecutable(QString path, MonitorInfo info, AppPosition pos);
	bool launchExecutable(QString path, MonitorInfo info, AppPosition pos, const QRect& zoneRect);

	bool launchURL(QString url, MonitorInfo info, AppPosition pos);
	bool launchURL(QString url, MonitorInfo info, AppPosition pos, const QRect& zoneRect);
};