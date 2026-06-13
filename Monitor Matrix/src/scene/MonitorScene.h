#pragma once
#include <qgraphicsscene.h>
#include <qlist.h>
#include <qmap.h>
#include <qrect.h>
#include "MonitorDropItem.h"
#include "../models/MonitorInfo.h"
#include "../models/Placement.h"

class MonitorScene : public QGraphicsScene {
	Q_OBJECT

private:
	void rebuildItems();

	QList<MonitorInfo> m_monitors;

	// key = placementId, not appId
	QMap<QString, Placement> m_placements;

	MonitorDropItem* findMonitor(const QString& deviceName) const;

	void clearConflictingPlacements(const QString deviceName, AppPosition newZone, const QString exceptPlacementId = QString());
	void clearConflictingPlacements(const QString deviceName, QRectF newRect, const QString exceptPlacementId = QString());

	
	void removePlacementVisual(const Placement& placement);

	bool m_hasOverlaps = false;

private slots:
	void onPlacementRequested(QString appId, QString appName, QString deviceName, AppPosition position);

	void onPlacementMoveRequested(QString placementId, QString appId, QString appName,
		QString deviceName, AppPosition position);

	void onPlacementCustomRectRequested(QString placementId, QString appId, QString appName,
		QString deviceName, QRectF zoneRect);

	void onPlacementRemoveRequested(QString placementId);

protected:
	void drawBackground(QPainter* painter, const QRectF& rect) override;
	void keyPressEvent(QKeyEvent* event) override;

public:
	MonitorScene(QObject* parent = nullptr);

	void setMonitors(QList<MonitorInfo> monitors);
	QList<Placement> placements();
	void clearPlacements();
	void addPlacement(const Placement& placement, const QString& appName = QString());

	QRectF rectForPosition(const QRectF& monitorRect, AppPosition position) const;
	QRectF rectForPlacement(const Placement& placement) const;

	void updateOverlapStates();

signals:
	void placementChanged();
	void overlapStateChanged(bool hasOverlaps, int overlapCount);
};