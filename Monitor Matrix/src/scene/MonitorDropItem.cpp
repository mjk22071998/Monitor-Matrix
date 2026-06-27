#include <qpainter.h>
#include <qgraphicssceneevent.h>
#include <qwidget.h>
#include <qstyleoption.h>
#include <qmimedata.h>
#include <qfont.h>
#include <qglobal.h>

#include "MonitorDropItem.h"
#include "../theme/AppTheme.h"

MonitorDropItem::MonitorDropItem(MonitorInfo info, QGraphicsItem* parent)
	: QGraphicsRectItem(info.geometry, parent)
{
	m_info = info;

	setAcceptDrops(true);
	setPen(QPen(AppTheme::palette().monitorBorder, 2.0));
	setBrush(AppTheme::palette().monitorFill);

	m_badgeBg = new QGraphicsRectItem(this);
	m_badgeBg->setBrush(AppTheme::palette().badgeFill);
	m_badgeBg->setPen(Qt::NoPen);
	m_badgeBg->setZValue(1000);
	m_badgeBg->setVisible(false);

	m_badgeText = new QGraphicsTextItem(this);
	m_badgeText->setDefaultTextColor(AppTheme::palette().badgeText);

	QFont f = m_badgeText->font();
	f.setBold(true);
	m_badgeText->setFont(f);
	m_badgeText->setZValue(2000);

	updateBadge();
}

void MonitorDropItem::updateDockTargetRects()
{
	m_dockTargetRects.clear();

	QRectF r = rect();

	qreal size = qMin(r.width(), r.height()) * 0.11;
	size = qBound<qreal>(34.0, size, 72.0);

	qreal gap = size * 0.22;

	qreal gridW = size * 3 + gap * 2;
	qreal gridH = size * 3 + gap * 2;

	qreal startX = r.center().x() - gridW / 2.0;
	qreal startY = r.center().y() - gridH / 2.0;

	auto cell = [&](int col, int row) {
		return QRectF(
			startX + col * (size + gap),
			startY + row * (size + gap),
			size,
			size
		);
		};

	m_dockTargetRects[AppPosition::TOP_LEFT] = cell(0, 0);
	m_dockTargetRects[AppPosition::TOP_HALF] = cell(1, 0);
	m_dockTargetRects[AppPosition::TOP_RIGHT] = cell(2, 0);

	m_dockTargetRects[AppPosition::LEFT_HALF] = cell(0, 1);
	m_dockTargetRects[AppPosition::FULL_SCREEN] = cell(1, 1);
	m_dockTargetRects[AppPosition::RIGHT_HALF] = cell(2, 1);

	m_dockTargetRects[AppPosition::BOTTOM_LEFT] = cell(0, 2);
	m_dockTargetRects[AppPosition::BOTTOM_HALF] = cell(1, 2);
	m_dockTargetRects[AppPosition::BOTTOM_RIGHT] = cell(2, 2);
}

QRectF MonitorDropItem::dockPreviewRect(AppPosition position) const
{
	QRectF r = rect();

	qreal w = r.width();
	qreal h = r.height();

	switch (position) {
	case AppPosition::TOP_LEFT:
		return QRectF(r.left(), r.top(), w * 0.5, h * 0.5);

	case AppPosition::TOP_HALF:
		return QRectF(r.left(), r.top(), w, h * 0.5);

	case AppPosition::TOP_RIGHT:
		return QRectF(r.left() + w * 0.5, r.top(), w * 0.5, h * 0.5);

	case AppPosition::LEFT_HALF:
		return QRectF(r.left(), r.top(), w * 0.5, h);

	case AppPosition::RIGHT_HALF:
		return QRectF(r.left() + w * 0.5, r.top(), w * 0.5, h);

	case AppPosition::BOTTOM_LEFT:
		return QRectF(r.left(), r.top() + h * 0.5, w * 0.5, h * 0.5);

	case AppPosition::BOTTOM_HALF:
		return QRectF(r.left(), r.top() + h * 0.5, w, h * 0.5);

	case AppPosition::BOTTOM_RIGHT:
		return QRectF(r.left() + w * 0.5, r.top() + h * 0.5, w * 0.5, h * 0.5);

	case AppPosition::FULL_SCREEN:
	default:
		return r;
	}
}

QRectF MonitorDropItem::defaultCustomRectAt(QPointF localPoint) const
{
	QRectF monitorRect = rect();

	qreal w = monitorRect.width() * 0.30;
	qreal h = monitorRect.height() * 0.25;

	w = qMax<qreal>(220.0, w);
	h = qMax<qreal>(140.0, h);

	w = qMin(w, monitorRect.width());
	h = qMin(h, monitorRect.height());

	QRectF customRect(
		localPoint.x() - w / 2.0,
		localPoint.y() - h / 2.0,
		w,
		h
	);

	if (customRect.left() < monitorRect.left()) {
		customRect.moveLeft(monitorRect.left());
	}

	if (customRect.top() < monitorRect.top()) {
		customRect.moveTop(monitorRect.top());
	}

	if (customRect.right() > monitorRect.right()) {
		customRect.moveRight(monitorRect.right());
	}

	if (customRect.bottom() > monitorRect.bottom()) {
		customRect.moveBottom(monitorRect.bottom());
	}

	return customRect;
}

bool MonitorDropItem::dockPositionFromPoint(QPointF point, AppPosition& outPosition)
{
	updateDockTargetRects();

	for (auto it = m_dockTargetRects.begin(); it != m_dockTargetRects.end(); ++it) {
		if (it.value().contains(point)) {
			outPosition = it.key();
			return true;
		}
	}

	return false;
}

void MonitorDropItem::setDockOverlayVisible(bool visible)
{
	m_showDockOverlay = visible;
	m_dragging = visible;

	if (!visible) {
		m_hasDockHover = false;
		m_zoneHighlight = QRectF();
	}

	update();
}

void MonitorDropItem::updateDockHover(QPointF point)
{
	AppPosition position;

	if (dockPositionFromPoint(point, position)) {
		m_hasDockHover = true;
		m_hoverDockPosition = position;
		m_zoneHighlight = dockPreviewRect(position);
	}
	else {
		m_hasDockHover = false;
		m_zoneHighlight = QRectF();
	}

	update();
}

void MonitorDropItem::updateBadge()
{
	if (!m_badgeBg || !m_badgeText) {
		return;
	}

	QRectF r = rect();

	QString indexText = m_info.model.isEmpty() ? m_info.deviceName : m_info.model;

	QFont tf = m_badgeText->font();
	qreal largeSize = qMax<qreal>(24.0, r.height() * 0.18);

	tf.setPointSizeF(largeSize);
	tf.setBold(true);

	m_badgeText->setFont(tf);
	m_badgeText->setPlainText(indexText);

	QRectF textRect = m_badgeText->boundingRect();

	qreal tx = r.right() - textRect.width() - 12;
	qreal ty = r.bottom() - textRect.height() - 8;

	m_badgeText->setPos(tx, ty);
	m_badgeBg->setBrush(AppTheme::palette().badgeFill);
	m_badgeText->setDefaultTextColor(AppTheme::palette().badgeText);
	m_badgeText->setZValue(2000);
}

void MonitorDropItem::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
	if (event->mimeData()->hasFormat("application/x-screendrop-appId")) {
		event->acceptProposedAction();
		setDockOverlayVisible(true);
		updateDockHover(event->pos());
	}
	else {
		event->ignore();
	}
}

void MonitorDropItem::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
	setDockOverlayVisible(false);
	event->accept();
}

void MonitorDropItem::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
	if (event->mimeData()->hasFormat("application/x-screendrop-appId")) {
		updateDockHover(event->pos());
		event->acceptProposedAction();
	}
	else {
		event->ignore();
	}
}

void MonitorDropItem::dropEvent(QGraphicsSceneDragDropEvent* event)
{
	setDockOverlayVisible(false);

	if (!event->mimeData()->hasFormat("application/x-screendrop-appId")) {
		event->ignore();
		return;
	}

	QString appId = QString::fromUtf8(event->mimeData()->data("application/x-screendrop-appId"));
	QString appName = QString::fromUtf8(event->mimeData()->data("application/x-screendrop-appName"));

	if (appName.isEmpty()) {
		appName = appId;
	}

	AppPosition dockPosition;

	if (dockPositionFromPoint(event->pos(), dockPosition)) {
		emit placementRequested(appId, appName, m_info.deviceName, dockPosition);
	}
	else {
		QRectF customRect = defaultCustomRectAt(event->pos());

		emit placementCustomRectRequested(
			QString(),
			appId,
			appName,
			m_info.deviceName,
			customRect
		);
	}

	event->acceptProposedAction();
	update();
}

void MonitorDropItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	setPen(QPen(AppTheme::palette().monitorBorder, 2.0));
	setBrush(AppTheme::palette().monitorFill);

	if (m_badgeBg) {
		m_badgeBg->setBrush(AppTheme::palette().badgeFill);
	}

	if (m_badgeText) {
		m_badgeText->setDefaultTextColor(AppTheme::palette().badgeText);
	}

	QGraphicsRectItem::paint(painter, option, widget);

	if (!m_zoneHighlight.isNull() && m_dragging) {
		painter->save();
		painter->setPen(Qt::NoPen);
		painter->setBrush(AppTheme::palette().dropHighlight);
		painter->drawRect(m_zoneHighlight);
		painter->restore();
	}

	if (m_showDockOverlay) {
		updateDockTargetRects();

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing);

		for (auto it = m_dockTargetRects.begin(); it != m_dockTargetRects.end(); ++it) {
			bool hovered = m_hasDockHover && it.key() == m_hoverDockPosition;

			QColor border = hovered
				? AppTheme::palette().dockHoverBorder
				: AppTheme::palette().dockBorder;

			QColor fill = hovered
				? AppTheme::palette().dockHoverFill
				: AppTheme::palette().dockFill;

			painter->setPen(QPen(border, hovered ? 2.5 : 1.5));
			painter->setBrush(fill);
			painter->drawRoundedRect(it.value(), 6, 6);

			QRectF iconRect = it.value().adjusted(8, 8, -8, -8);
			QRectF screenRect = iconRect;

			painter->setPen(QPen(AppTheme::palette().dockScreenOutline, 1.2));
			painter->setBrush(Qt::NoBrush);
			painter->drawRect(screenRect);

			QRectF fillRect = dockPreviewRect(it.key());
			QRectF monitorRect = rect();

			qreal xRatio = (fillRect.left() - monitorRect.left()) / monitorRect.width();
			qreal yRatio = (fillRect.top() - monitorRect.top()) / monitorRect.height();
			qreal wRatio = fillRect.width() / monitorRect.width();
			qreal hRatio = fillRect.height() / monitorRect.height();

			QRectF miniFill(
				screenRect.left() + screenRect.width() * xRatio,
				screenRect.top() + screenRect.height() * yRatio,
				screenRect.width() * wRatio,
				screenRect.height() * hRatio
			);

			painter->setPen(Qt::NoPen);
			painter->setBrush(hovered
				? AppTheme::palette().dockPreviewHoverFill
				: AppTheme::palette().dockPreviewFill);
			painter->drawRect(miniFill);
		}

		painter->restore();
	}
}

MonitorInfo MonitorDropItem::getMonitorInfo()
{
	return m_info;
}

void MonitorDropItem::addPlacementVisual(AppPosition zone, QString placementId, QString appId, QString appName)
{
	QRectF zoneRect = dockPreviewRect(zone);

	PlacedItem* placedItem = new PlacedItem(placementId, appId, appName, zoneRect, this);

	connect(placedItem, &PlacedItem::moveRequested,
		this, &MonitorDropItem::placementMoveRequested);

	connect(placedItem, &PlacedItem::customRectRequested,
		this, &MonitorDropItem::placementCustomRectRequested);

	connect(placedItem, &PlacedItem::removeRequested,
		this, [this](QString placementId) {
			emit placementRemoveRequested(placementId);
		});

	m_allPlacedItem.append(placedItem);

	// Keep this map only as a best-effort reference for legacy removal.
	// Multiple placements can now share the same dock position, so removal should prefer placementId.
	m_placedItems.insert(zone, placedItem);
}

void MonitorDropItem::addPlacementVisual(const QRectF& zoneRect, QString placementId, QString appId, QString appName)
{
	PlacedItem* placedItem = new PlacedItem(placementId, appId, appName, zoneRect, this);

	connect(placedItem, &PlacedItem::moveRequested,
		this, &MonitorDropItem::placementMoveRequested);

	connect(placedItem, &PlacedItem::customRectRequested,
		this, &MonitorDropItem::placementCustomRectRequested);

	connect(placedItem, &PlacedItem::removeRequested,
		this, [this](QString placementId) {
			emit placementRemoveRequested(placementId);
		});

	m_allPlacedItem.append(placedItem);
}

void MonitorDropItem::removePlacementVisual(AppPosition zone)
{
	if (m_placedItems.contains(zone)) {
		auto* placedItem = m_placedItems.take(zone);
		m_allPlacedItem.removeOne(placedItem);
		delete placedItem;
	}
}

void MonitorDropItem::removePlacementVisualById(const QString& placementId)
{
	PlacedItem* found = nullptr;

	for (auto* item : m_allPlacedItem) {
		if (item->getPlacementId() == placementId) {
			found = item;
			break;
		}
	}

	if (!found) {
		return;
	}

	m_allPlacedItem.removeOne(found);

	for (auto it = m_placedItems.begin(); it != m_placedItems.end();) {
		if (it.value() == found) {
			it = m_placedItems.erase(it);
		}
		else {
			++it;
		}
	}

	delete found;
}

void MonitorDropItem::removeAllPlacements()
{
	for (auto* item : m_allPlacedItem) {
		delete item;
	}

	m_allPlacedItem.clear();
	m_placedItems.clear();
}

void MonitorDropItem::setPlacementOverlapState(const QString& placementId, bool hasOverlap)
{
	for (auto* item : m_allPlacedItem) {
		if (item->getPlacementId() == placementId) {
			item->setHasOverlap(hasOverlap);
			return;
		}
	}
}
