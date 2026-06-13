#include "URLStrategy.h"
#include "../services/LaunchService.h"
#include <qdebug.h>
#include <qurl.h>

URLStrategy::URLStrategy(const QString& path) {
	m_url = path.trimmed();
}

void URLStrategy::launch(MonitorInfo& monitor, AppPosition position, const QRect& zoneRect)
{
	if (!ensureLaunchService()) {
		qWarning() << "Cannot launch URL because LaunchService is unavailable";
		return;
	}

	if (m_url.isEmpty()) {
		qWarning() << "URL is empty";
		return;
	}

	QUrl url(m_url);
	if (!url.isValid() || url.scheme().isEmpty()) {
		QUrl assumed("http://" + m_url);
		if (!assumed.isValid()) {
			qWarning() << "Invalid URL:" << m_url;
			return;
		}
		m_url = assumed.toString();
	}

	if (zoneRect.isNull()) {
		p_launchService->launchURL(m_url, monitor, position);
	}
	else {
		p_launchService->launchURL(m_url, monitor, position, zoneRect);
	}
}