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

	QString launchUrl = m_url;
	QUrl url(launchUrl);
	if (!url.isValid() || url.scheme().isEmpty()) {
		QUrl assumed("http://" + launchUrl);
		if (!assumed.isValid()) {
			qWarning() << "Invalid URL:" << launchUrl;
			return;
		}
		launchUrl = assumed.toString();
	}

	if (zoneRect.isNull()) {
		p_launchService->launchURL(launchUrl, monitor, position);
	}
	else {
		p_launchService->launchURL(launchUrl, monitor, position, zoneRect);
	}
}
