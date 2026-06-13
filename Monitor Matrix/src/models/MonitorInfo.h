#pragma once
#include <qstring.h>
#include <qrect.h>

struct MonitorInfo {
	QString deviceName;
	QString model;
	QRect geometry;
	QRect availableGeometry;
	bool primary = false;
};
