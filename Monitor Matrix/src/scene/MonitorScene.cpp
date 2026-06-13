#include "MonitorScene.h"
#include <qgraphicsitem.h>
#include <qpainter.h>
#include <qobject.h>
#include <qset.h>
#include <quuid.h>
#include <qkeysequence.h>
#include <qevent.h>

MonitorDropItem* MonitorScene::findMonitor(const QString &deviceName) const
{
	for (auto* item : items()) {
		MonitorDropItem* monitor = dynamic_cast<MonitorDropItem*>(item);
		if (monitor && monitor->getMonitorInfo().deviceName == deviceName) {
			return monitor;
		}
	}

	return nullptr;
}

QRectF MonitorScene::rectForPosition(const QRectF& monitorRect, AppPosition position) const
{
	qreal w = monitorRect.width();
	qreal h = monitorRect.height();

	switch (position) {
	case AppPosition::TOP_LEFT:
		return QRectF(monitorRect.left(), monitorRect.top(), w * 0.5, h * 0.5);

	case AppPosition::TOP_HALF:
		return QRectF(monitorRect.left(), monitorRect.top(), w, h * 0.5);

	case AppPosition::TOP_RIGHT:
		return QRectF(monitorRect.left() + w * 0.5, monitorRect.top(), w * 0.5, h * 0.5);

	case AppPosition::LEFT_HALF:
		return QRectF(monitorRect.left(), monitorRect.top(), w * 0.5, h);

	case AppPosition::RIGHT_HALF:
		return QRectF(monitorRect.left() + w * 0.5, monitorRect.top(), w * 0.5, h);

	case AppPosition::BOTTOM_LEFT:
		return QRectF(monitorRect.left(), monitorRect.top() + h * 0.5, w * 0.5, h * 0.5);

	case AppPosition::BOTTOM_HALF:
		return QRectF(monitorRect.left(), monitorRect.top() + h * 0.5, w, h * 0.5);

	case AppPosition::BOTTOM_RIGHT:
		return QRectF(monitorRect.left() + w * 0.5, monitorRect.top() + h * 0.5, w * 0.5, h * 0.5);

	case AppPosition::FULL_SCREEN:
	default:
		return monitorRect;
	}
}

QRectF MonitorScene::rectForPlacement(const Placement& placement) const
{
	if (placement.customRect) {
		return QRectF(placement.zoneRect);
	}

	auto* monitor = findMonitor(placement.monitor.deviceName);
	if (!monitor) {
		return QRectF();
	}

	return rectForPosition(monitor->rect(), placement.position);
}

void MonitorScene::removePlacementVisual(const Placement& placement)
{
	auto* monitor = findMonitor(placement.monitor.deviceName);
	if (!monitor) {
		return;
	}

	monitor->removePlacementVisualById(placement.placementId);
}

void MonitorScene::updateOverlapStates()
{
	QSet<QString> overlappingIds;

	auto keys = m_placements.keys();

	for (int i = 0; i < keys.size(); ++i) {
		const Placement& a = m_placements[keys[i]];
		QRectF rectA = rectForPlacement(a);

		if (rectA.isNull()) {
			continue;
		}

		for (int j = i + 1; j < keys.size(); ++j) {
			const Placement& b = m_placements[keys[j]];

			if (a.monitor.deviceName != b.monitor.deviceName) {
				continue;
			}

			QRectF rectB = rectForPlacement(b);

			if (rectB.isNull()) {
				continue;
			}

			if (rectA.intersects(rectB)) {
				overlappingIds.insert(a.placementId);
				overlappingIds.insert(b.placementId);
			}
		}
	}

	for (auto it = m_placements.begin(); it != m_placements.end(); ++it) {
		const Placement& placement = it.value();

		auto* monitor = findMonitor(placement.monitor.deviceName);
		if (!monitor) {
			continue;
		}

		monitor->setPlacementOverlapState(
			placement.placementId,
			overlappingIds.contains(placement.placementId)
		);
	}

	int invalidPlacementCount = overlappingIds.size();
	bool newHasOverlaps = invalidPlacementCount > 0;

	m_hasOverlaps = newHasOverlaps;

	emit overlapStateChanged(m_hasOverlaps, invalidPlacementCount);
}

void MonitorScene::onPlacementRequested(QString appId, QString appName, QString deviceName, AppPosition position)
{
	auto* monitor = findMonitor(deviceName);

	MonitorInfo mi;
	if (monitor) {
		mi = monitor->getMonitorInfo();
	}

	Placement placement(appId, mi, position);

	if (placement.placementId.isEmpty()) {
		placement.placementId = QUuid::createUuid().toString(QUuid::WithoutBraces);
	}

	m_placements.insert(placement.placementId, placement);

	if (monitor) {
		monitor->addPlacementVisual(position, placement.placementId, appId, appName);
	}

	updateOverlapStates();
	emit placementChanged();
}

void MonitorScene::onPlacementMoveRequested(QString placementId, QString appId, QString appName,
	QString deviceName, AppPosition position)
{
	if (placementId.isEmpty()) {
		placementId = QUuid::createUuid().toString(QUuid::WithoutBraces);
	}

	if (m_placements.contains(placementId)) {
		Placement old = m_placements.take(placementId);
		removePlacementVisual(old);
	}

	auto* monitor = findMonitor(deviceName);

	MonitorInfo mi;
	if (monitor) {
		mi = monitor->getMonitorInfo();
	}

	Placement placement(appId, mi, position);
	placement.placementId = placementId;

	m_placements.insert(placement.placementId, placement);

	if (monitor) {
		monitor->addPlacementVisual(position, placement.placementId, appId, appName);
	}

	updateOverlapStates();
	emit placementChanged();
}

void MonitorScene::onPlacementCustomRectRequested(QString placementId, QString appId, QString appName,
	QString deviceName, QRectF zoneRect)
{
	if (placementId.isEmpty()) {
		placementId = QUuid::createUuid().toString(QUuid::WithoutBraces);
	}

	if (m_placements.contains(placementId)) {
		Placement old = m_placements.take(placementId);
		removePlacementVisual(old);
	}

	auto* monitor = findMonitor(deviceName);

	MonitorInfo mi;
	if (monitor) {
		mi = monitor->getMonitorInfo();
	}

	Placement placement(appId, mi, zoneRect.toRect());
	placement.placementId = placementId;

	m_placements.insert(placement.placementId, placement);

	if (monitor) {
		monitor->addPlacementVisual(zoneRect, placement.placementId, appId, appName);
	}

	updateOverlapStates();
	emit placementChanged();
}

void MonitorScene::onPlacementRemoveRequested(QString placementId)
{
	if (!m_placements.contains(placementId)) {
		return;
	}

	Placement old = m_placements.take(placementId);
	removePlacementVisual(old);

	updateOverlapStates();
	emit placementChanged();
}

void MonitorScene::drawBackground(QPainter* painter, const QRectF& rect)
{
	painter->fillRect(rect, QColor(9, 9, 16));
}

MonitorScene::MonitorScene(QObject* parent) : QGraphicsScene(parent)
{
	setSceneRect(-10000, -10000, 20000, 20000);
}

void MonitorScene::setMonitors(QList<MonitorInfo> monitors)
{
	m_monitors = monitors;
	rebuildItems();

	updateOverlapStates();
	emit placementChanged();
}

QList<Placement> MonitorScene::placements()
{
	return m_placements.values();
}

void MonitorScene::clearPlacements()
{
	for (auto* item : items()) {
		MonitorDropItem* monitorItem = dynamic_cast<MonitorDropItem*>(item);
		if (monitorItem) {
			monitorItem->removeAllPlacements();
		}
	}

	m_placements.clear();

	updateOverlapStates();
	emit placementChanged();
}

void MonitorScene::addPlacement(const Placement& placement, const QString& appName)
{
	Placement fixedPlacement = placement;

	if (fixedPlacement.placementId.isEmpty()) {
		fixedPlacement.placementId = QUuid::createUuid().toString(QUuid::WithoutBraces);
	}

	m_placements.insert(fixedPlacement.placementId, fixedPlacement);

	auto* monitor = findMonitor(fixedPlacement.monitor.deviceName);

	if (monitor) {
		if (fixedPlacement.customRect) {
			monitor->addPlacementVisual(
				QRectF(fixedPlacement.zoneRect),
				fixedPlacement.placementId,
				fixedPlacement.appId,
				appName
			);
		}
		else {
			monitor->addPlacementVisual(
				fixedPlacement.position,
				fixedPlacement.placementId,
				fixedPlacement.appId,
				appName
			);
		}
	}

	updateOverlapStates();
	emit placementChanged();
}

void MonitorScene::rebuildItems()
{
	clear();

	int idx = 1;

	for (MonitorInfo& monitor : m_monitors) {
		monitor.model = QString::number(idx);

		MonitorDropItem* item = new MonitorDropItem(monitor);

		connect(item, &MonitorDropItem::placementRequested,
			this, &MonitorScene::onPlacementRequested);

		connect(item, &MonitorDropItem::placementMoveRequested,
			this, &MonitorScene::onPlacementMoveRequested);

		connect(item, &MonitorDropItem::placementCustomRectRequested,
			this, &MonitorScene::onPlacementCustomRectRequested);

		connect(item, &MonitorDropItem::placementRemoveRequested,
			this, &MonitorScene::onPlacementRemoveRequested);

		addItem(item);
		idx++;
	}

	for (auto* item : items()) {
		MonitorDropItem* monitorItem = dynamic_cast<MonitorDropItem*>(item);
		if (monitorItem) {
			monitorItem->updateBadge();
		}
	}

	if (!m_monitors.isEmpty()) {
		QRectF bounds;

		for (MonitorInfo& monitor : m_monitors) {
			bounds = bounds.united(monitor.geometry);
		}

		bounds.adjust(-50, -50, 50, 50);
		setSceneRect(bounds);
	}
}

void MonitorScene::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
		for (QGraphicsItem* item : selectedItems()) {
			PlacedItem* placedItem = dynamic_cast<PlacedItem*>(item);
			if (placedItem) {
				onPlacementRemoveRequested(placedItem->getPlacementId());
				event->accept();
				return;
			}
		}
	}

	QGraphicsScene::keyPressEvent(event);
}