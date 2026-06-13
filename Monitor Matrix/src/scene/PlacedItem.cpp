#include "PlacedItem.h"
#include "MonitorDropItem.h"

#include <qcursor.h>
#include <qfont.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsview.h>
#include <qpainter.h>
#include <qpen.h>
#include <qstyleoption.h>
#include <qdebug.h>
#include <qtimer.h>

#include "PlacedItemHandles.h"
#include "PlacedItemResize.h"
#include "PlacedItemSnap.h"

PlacedItem::PlacedItem(const QString& placementId,
    const QString& appId,
    const QString& appName,
    const QRectF& zoneRect,
    QGraphicsItem* parent)
    : QGraphicsRectItem(QRectF(0, 0, zoneRect.width(), zoneRect.height()), parent)
    , m_placementId(placementId)
    , m_appId(appId)
    , m_appName(appName)
{
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setFlag(ItemIsFocusable);

    setAcceptDrops(false);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);

    setPen(QPen(QColor(32, 210, 180, 230), 2.0));
    setBrush(QColor(32, 210, 180, 30));

    m_label = new QGraphicsTextItem(appName, this);
    m_label->setDefaultTextColor(QColor(226, 226, 240));

    QFont font("Arial", 9);
    font.setBold(true);

    m_label->setFont(font);
    m_label->setTextWidth(zoneRect.width() - 8);
    m_label->setFlag(ItemIgnoresTransformations);
    m_label->setPos(4, 4);

    setPos(zoneRect.topLeft());
}

QString PlacedItem::getPlacementId() const
{
    return m_placementId;
}

QString PlacedItem::getAppId() const
{
    return m_appId;
}

QString PlacedItem::getAppName() const
{
    return m_appName;
}

void PlacedItem::setHasOverlap(bool hasOverlap)
{
    m_hasOverlap = hasOverlap;
    update();
}

bool PlacedItem::hasOverlap() const
{
    return m_hasOverlap;
}

void PlacedItem::paint(QPainter* painter,
    const QStyleOptionGraphicsItem*,
    QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QColor borderColor = m_hasOverlap
        ? QColor(255, 80, 80, 255)
        : QColor(32, 210, 180, 230);

    QColor fillColor = m_hasOverlap
        ? QColor(255, 80, 80, 45)
        : QColor(32, 210, 180, 30);

    painter->setPen(QPen(borderColor, 2.0));
    painter->setBrush(fillColor);
    painter->drawRect(rect());

    if (!isSelected()) {
        return;
    }

    QTransform itemToDevice = painter->transform();
    const qreal r = placedItemHandleRadiusFromTransform(itemToDevice, HANDLE_PX);

    QColor handleColor = m_hasOverlap
        ? QColor(255, 80, 80, 255)
        : QColor(32, 210, 180, 255);

    painter->setPen(QPen(handleColor, 2.0));
    painter->setBrush(QColor(handleColor.red(), handleColor.green(), handleColor.blue(), 180));

    const PlacedItemHandlePoints hp = placedItemHandlePoints(rect());
    const QPointF points[] = {
        hp.tl, hp.tm, hp.tr,
        hp.ml,        hp.mr,
        hp.bl, hp.bm, hp.br
    };

    for (const QPointF& point : points) {
        painter->drawEllipse(point, r, r);
    }
}

QVariant PlacedItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        update();
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void PlacedItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (scene()) {
        scene()->clearSelection();
    }

    QGraphicsRectItem::mousePressEvent(event);

    setSelected(true);
    setFocus();

    QTransform itemToDevice;
    if (scene() && !scene()->views().isEmpty()) {
        itemToDevice = deviceTransform(scene()->views().first()->viewportTransform());
    }

    Handle h = placedItemHitTest(rect(), event->pos(), itemToDevice, HANDLE_HIT_PX);

    if (h != Handle::None && isSelected()) {
        m_resizing = true;
        m_activeHandle = h;
        m_origParentRect = QRectF(mapToParent(rect().topLeft()), rect().size());

        m_resizeStartParent = parentItem()
            ? parentItem()->mapFromScene(event->scenePos())
            : mapFromScene(event->scenePos());

        setCursor(cursorForPlacedItemHandle(h));
        event->accept();
    }
    else {
        m_resizing = false;
        m_activeHandle = Handle::None;
        m_hasMoved = false;
        setCursor(Qt::ClosedHandCursor);
        p_lastHighlightedMonitor = nullptr;
    }
}

void PlacedItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_resizing) {
        QPointF cur = parentItem()
            ? parentItem()->mapFromScene(event->scenePos())
            : mapFromScene(event->scenePos());

        QRectF r = resizedParentRectForHandle(
            m_origParentRect,
            m_resizeStartParent,
            cur,
            m_activeHandle
        );

        MonitorDropItem* parentMonitor = dynamic_cast<MonitorDropItem*>(parentItem());

        bool showVerticalGuide = false;
        bool showHorizontalGuide = false;
        qreal verticalGuideX = 0.0;
        qreal horizontalGuideY = 0.0;

        if (parentMonitor) {
            QRectF monitorRect = parentMonitor->rect();

            applyMonitorCenterSnap(
                r,
                monitorRect,
                m_activeHandle,
                showVerticalGuide,
                showHorizontalGuide,
                verticalGuideX,
                horizontalGuideY
            );

            snapAndBlockResizeAgainstOtherItems(
                r,
                m_origParentRect,
                m_activeHandle,
                this,
                showVerticalGuide,
                showHorizontalGuide,
                verticalGuideX,
                horizontalGuideY
            );

            clampResizeRectToBounds(r, monitorRect, m_activeHandle);
            updateResizeGuides(parentMonitor, showVerticalGuide, showHorizontalGuide, verticalGuideX, horizontalGuideY);
        }

        enforceMinSizeForHandle(r, m_activeHandle);

        prepareGeometryChange();
        setPos(r.topLeft());
        setRect(QRectF(0, 0, r.width(), r.height()));
        m_label->setTextWidth(qMax<qreal>(0.0, r.width() - 8.0));

        event->accept();
        return;
    }

    QGraphicsRectItem::mouseMoveEvent(event);
    m_hasMoved = true;

    QPointF scenePos = mapToScene(event->pos());
    MonitorDropItem* targetMonitor = nullptr;

    for (QGraphicsItem* it : scene()->items(scenePos)) {
        QGraphicsItem* cur = it;

        while (cur) {
            targetMonitor = dynamic_cast<MonitorDropItem*>(cur);

            if (targetMonitor) {
                break;
            }

            cur = cur->parentItem();
        }

        if (targetMonitor) {
            break;
        }
    }

    if (p_lastHighlightedMonitor && p_lastHighlightedMonitor != targetMonitor) {
        p_lastHighlightedMonitor->setDockOverlayVisible(false);
    }

    if (targetMonitor) {
        QPointF localPos = targetMonitor->mapFromScene(scenePos);
        targetMonitor->setDockOverlayVisible(true);
        targetMonitor->updateDockHover(localPos);
        p_lastHighlightedMonitor = targetMonitor;
    }
    else {
        if (p_lastHighlightedMonitor) {
            p_lastHighlightedMonitor->setDockOverlayVisible(false);
            p_lastHighlightedMonitor = nullptr;
        }
    }
}

void PlacedItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    hideCenterGuides();
    clearDragHighlight();

    if (m_resizing) {
        m_resizing = false;
        m_activeHandle = Handle::None;
        setCursor(Qt::OpenHandCursor);

        setSelected(true);
        setFocus();

        event->accept();

        MonitorDropItem* targetMonitor = dynamic_cast<MonitorDropItem*>(parentItem());

        if (targetMonitor) {
            QRectF rectInMonitor = clampRectToBounds(
                mapRectToParent(rect()),
                targetMonitor->rect()
            );

            emit customRectRequested(
                m_placementId,
                m_appId,
                m_appName,
                targetMonitor->getMonitorInfo().deviceName,
                rectInMonitor
            );
        }

        return;
    }

    const bool didMove = m_hasMoved;
    m_hasMoved = false;

    QGraphicsRectItem::mouseReleaseEvent(event);

    setSelected(true);
    setFocus();

    setCursor(Qt::OpenHandCursor);

    if (!didMove) {
        return;
    }

    QPointF scenePos = mapToScene(event->pos());
    MonitorDropItem* targetMonitor = nullptr;

    for (QGraphicsItem* it : scene()->items(scenePos)) {
        targetMonitor = dynamic_cast<MonitorDropItem*>(it);

        if (targetMonitor) {
            break;
        }

        QGraphicsItem* cur = it->parentItem();

        while (cur) {
            targetMonitor = dynamic_cast<MonitorDropItem*>(cur);

            if (targetMonitor) {
                if (!targetMonitor->rect().contains(targetMonitor->mapFromScene(scenePos))) {
                    targetMonitor = nullptr;
                }

                break;
            }

            cur = cur->parentItem();
        }

        if (targetMonitor) {
            break;
        }
    }

    if (!targetMonitor) {
        const QString placementIdCopy = m_placementId;

        QTimer::singleShot(0, this, [this, placementIdCopy]() {
            emit removeRequested(placementIdCopy);
            });

        return;
    }

    QPointF localPos = targetMonitor->mapFromScene(scenePos);

    AppPosition dockPosition;

    if (targetMonitor->dockPositionFromPoint(localPos, dockPosition)) {
        emit moveRequested(
            m_placementId,
            m_appId,
            m_appName,
            targetMonitor->getMonitorInfo().deviceName,
            dockPosition
        );
    }
    else {
        QRectF rectInTarget = targetMonitor->mapRectFromScene(mapRectToScene(rect()));
        rectInTarget = clampRectToBounds(rectInTarget, targetMonitor->rect());

        emit customRectRequested(
            m_placementId,
            m_appId,
            m_appName,
            targetMonitor->getMonitorInfo().deviceName,
            rectInTarget
        );
    }
}

QRectF PlacedItem::boundingRect() const
{
    return QGraphicsRectItem::boundingRect().adjusted(-12, -12, 12, 12);
}

void PlacedItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    if (!isSelected()) {
        setCursor(Qt::OpenHandCursor);
        return;
    }

    QTransform itemToDevice;
    if (scene() && !scene()->views().isEmpty()) {
        itemToDevice = deviceTransform(scene()->views().first()->viewportTransform());
    }

    setCursor(cursorForPlacedItemHandle(placedItemHitTest(rect(), event->pos(), itemToDevice, HANDLE_HIT_PX)));
}

void PlacedItem::clearDragHighlight()
{
    if (p_lastHighlightedMonitor) {
        p_lastHighlightedMonitor->setDockOverlayVisible(false);
        p_lastHighlightedMonitor = nullptr;
    }
}

void PlacedItem::updateResizeGuides(
    MonitorDropItem* monitor,
    bool showVertical,
    bool showHorizontal,
    qreal verticalX,
    qreal horizontalY
)
{
    if (!monitor) {
        hideCenterGuides();
        return;
    }

    QRectF monitorRect = monitor->rect();

    if (!m_verticalCenterGuide) {
        m_verticalCenterGuide = new QGraphicsLineItem(monitor);
        m_verticalCenterGuide->setPen(QPen(QColor(255, 220, 80, 220), 1.5, Qt::DashLine));
        m_verticalCenterGuide->setZValue(9999);
    }

    if (!m_horizontalCenterGuide) {
        m_horizontalCenterGuide = new QGraphicsLineItem(monitor);
        m_horizontalCenterGuide->setPen(QPen(QColor(255, 220, 80, 220), 1.5, Qt::DashLine));
        m_horizontalCenterGuide->setZValue(9999);
    }

    m_verticalCenterGuide->setLine(
        verticalX,
        monitorRect.top(),
        verticalX,
        monitorRect.bottom()
    );

    m_horizontalCenterGuide->setLine(
        monitorRect.left(),
        horizontalY,
        monitorRect.right(),
        horizontalY
    );

    m_verticalCenterGuide->setVisible(showVertical);
    m_horizontalCenterGuide->setVisible(showHorizontal);
}

void PlacedItem::hideCenterGuides()
{
    if (m_verticalCenterGuide) {
        m_verticalCenterGuide->setVisible(false);
    }

    if (m_horizontalCenterGuide) {
        m_horizontalCenterGuide->setVisible(false);
    }
}