#include "MonitorService.h"
#include <qapplication.h>
#include <qscreen.h>

MonitorService::MonitorService(QObject* parent)
{
	refresh();
	connect(qApp, &QApplication::screenAdded, this, [this](QScreen*) {refresh(); emit monitorsChanged(); });
	connect(qApp, &QApplication::screenRemoved, this, [this](QScreen*) {refresh(); emit monitorsChanged(); });
}

void MonitorService::refresh()
{
	m_monitors.clear();
	const QList<QScreen*> screens = QApplication::screens();
	for (QScreen* screen : screens) {
		MonitorInfo info;
		info.deviceName = screen->name();
		info.model = screen->model();
		info.geometry = screen->geometry();
		info.availableGeometry = screen->availableGeometry();
		info.primary = (screen == QApplication::primaryScreen());
		m_monitors.append(info);
	}
}



QList<MonitorInfo> MonitorService::getCurrentMonitors()
{
	return m_monitors;
}
