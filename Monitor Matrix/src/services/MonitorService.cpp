#include "MonitorService.h"

#include "../platform/windows/WindowsDisplayTopology.h"

#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

QString rectToString(const QRect& rect)
{
	return QString("x=%1 y=%2 w=%3 h=%4")
		.arg(rect.x())
		.arg(rect.y())
		.arg(rect.width())
		.arg(rect.height());
}

}

MonitorService::MonitorService(QObject* parent)
	: QObject(parent)
{
	if (QCoreApplication::instance())
		QCoreApplication::instance()->installNativeEventFilter(this);

	refresh();
}

MonitorService::~MonitorService()
{
	if (QCoreApplication::instance())
		QCoreApplication::instance()->removeNativeEventFilter(this);
}

void MonitorService::refresh()
{
	QString errorMessage;
	m_monitors = WindowsDisplayTopology::getMonitors(&errorMessage);

	if (!errorMessage.isEmpty()) {
		qWarning().noquote()
			<< "Windows monitor refresh warning:" << errorMessage;
	}

	qInfo() << "Refreshing monitors from Windows. count=" << m_monitors.size();

	for (const MonitorInfo& info : m_monitors) {
		qInfo().noquote()
			<< "Windows monitor"
			<< "name=" << info.deviceName
			<< "model=" << info.model
			<< "geometry=" << rectToString(info.geometry)
			<< "available=" << rectToString(info.availableGeometry)
			<< "primary=" << info.primary;
	}
}

QList<MonitorInfo> MonitorService::getCurrentMonitors()
{
	return m_monitors;
}

void MonitorService::refreshMonitors()
{
	refresh();
	emit monitorsChanged();
}

bool MonitorService::nativeEventFilter(const QByteArray& eventType,
	void* message,
	qintptr* result)
{
	Q_UNUSED(eventType);
	Q_UNUSED(result);

#ifdef Q_OS_WIN
	MSG* msg = static_cast<MSG*>(message);

	if (!msg)
		return false;

	switch (msg->message) {
	case WM_DISPLAYCHANGE:
	case WM_DEVICECHANGE:
	case WM_SETTINGCHANGE:
		qInfo() << "Windows display-related message received:" << msg->message;
		refreshMonitors();
		break;
	default:
		break;
	}
#else
	Q_UNUSED(message);
#endif

	return false;
}
