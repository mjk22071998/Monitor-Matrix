#pragma once
#include <qgraphicsitem.h>
#include <qpoint.h>
#include <qmap.h>
#include <qrect.h>
#include "PlacedItem.h"
#include "../enums/AppPositionEnum.h"
#include "../strategies/LaunchStrategy.h"
#include "../models/MonitorInfo.h"

class MonitorDropItem : public QObject, public QGraphicsRectItem {
	Q_OBJECT

private:
	MonitorInfo m_info;

	QRectF m_zoneHighlight;
	bool m_dragging = false;

	QMap<AppPosition, PlacedItem*> m_placedItems;
	QList<PlacedItem*> m_allPlacedItem;

	QGraphicsRectItem* m_badgeBg = nullptr;
	QGraphicsTextItem* m_badgeText = nullptr;

	bool m_showDockOverlay = false;
	bool m_hasDockHover = false;
	AppPosition m_hoverDockPosition = AppPosition::FULL_SCREEN;

	QMap<AppPosition, QRectF> m_dockTargetRects;

	void updateDockTargetRects();
	QRectF dockPreviewRect(AppPosition position) const;
	QRectF defaultCustomRectAt(QPointF localPoint) const;

protected:
	void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
	void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
	void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
	void dropEvent(QGraphicsSceneDragDropEvent* event) override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

public:
	explicit MonitorDropItem(MonitorInfo info, QGraphicsItem* parent = nullptr);

	void updateBadge();

	MonitorInfo getMonitorInfo();

	bool dockPositionFromPoint(QPointF point, AppPosition& outPosition);
	void setDockOverlayVisible(bool visible);
	void updateDockHover(QPointF point);

	void addPlacementVisual(AppPosition zone, QString placementId, QString appId, QString appName);
	void addPlacementVisual(const QRectF& zoneRect, QString placementId, QString appId, QString appName);

	void removePlacementVisual(AppPosition zone);
	void removePlacementVisualById(const QString& placementId);
	void removeAllPlacements();

	void setPlacementOverlapState(const QString& placementId, bool hasOverlap);

signals:
	void placementRequested(QString appId, QString appName, QString deviceName, AppPosition position);

	void placementMoveRequested(QString placementId, QString appId, QString appName,
		QString deviceName, AppPosition position);

	void placementCustomRectRequested(QString placementId, QString appId, QString appName,
		QString deviceName, QRectF zoneRect);

	void placementRemoveRequested(QString placementId);
};