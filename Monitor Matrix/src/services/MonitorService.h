#pragma once
#include <qabstractnativeeventfilter.h>
#include <qobject.h>
#include <qlist.h>
#include "../models/MonitorInfo.h"

class MonitorService : public QObject, public QAbstractNativeEventFilter {
	Q_OBJECT;
private:
	QList<MonitorInfo> m_monitors;
	void refresh();

public:
	explicit MonitorService(QObject* parent = nullptr);
	~MonitorService() override;
	QList<MonitorInfo> getCurrentMonitors();
	void refreshMonitors();
	bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
signals:
	void monitorsChanged();
};
