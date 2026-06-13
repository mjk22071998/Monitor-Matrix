#pragma once
#include <qobject.h>
#include <qlist.h>
#include <qscreen.h>
#include "../models/MonitorInfo.h"

class MonitorService : public QObject {
	Q_OBJECT;
private:
	QList<MonitorInfo> m_monitors;
	void refresh();

public:
	explicit MonitorService(QObject* parent = nullptr);
	QList<MonitorInfo> getCurrentMonitors();
signals:
	void monitorsChanged();
};