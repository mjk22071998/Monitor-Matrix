#include "PlacedItem.h"
#include "MonitorDropItem.h"

#include <qcursor.h>
#include <qfont.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsview.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpen.h>
#include <qstyleoption.h>
#include <qdebug.h>
#include <qtimer.h>

#include "PlacedItemHandles.h"
#include "PlacedItemResize.h"
#include "PlacedItemSnap.h"
#include "../theme/AppTheme.h"

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

    setPen(QPen(AppTheme::palette().placementBorder, 2.0));
    setBrush(AppTheme::palette().placementFill);

    m_label = new QGraphicsTextItem(appName, this);
    m_label->setDefaultTextColor(AppTheme::palette().placementText);

    QFont font(AppTheme::fontFamily(), 9);
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

    if (m_label) {
        m_label->setDefaultTextColor(AppTheme::palette().placementText);
    }

    QColor borderColor = m_hasOverlap
        ? AppTheme::palette().placementOverlapBorder
        : AppTheme::palette().placementBorder;

    QColor fillColor = m_hasOverlap
        ? AppTheme::palette().placementOverlapFill
        : AppTheme::palette().placementFill;

    painter->setPen(QPen(borderColor, 2.0));
    painter->setBrush(fillColor);
    painter->drawRect(rect());

    if (!isSelected()) {
        return;
    }

    QTransform itemToDevice = painter->transform();
    const qreal r = placedItemHandleRadiusFromTransform(itemToDevice, HANDLE_PX);

    QColor handleColor = m_hasOverlap
        ? AppTheme::palette().placementOverlapBorder
        : AppTheme::palette().dockHoverBorder;

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
        updateDragGuides(targetMonitor);
    }
    else {
        if (p_lastHighlightedMonitor) {
            p_lastHighlightedMonitor->setDockOverlayVisible(false);
            p_lastHighlightedMonitor = nullptr;
        }

        hideCenterGuides();
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
    return QGraphicsRectItem::boundingRect().adjusted(
        -HANDLE_MARGIN_ITEM,
        -HANDLE_MARGIN_ITEM,
        HANDLE_MARGIN_ITEM,
        HANDLE_MARGIN_ITEM
    );
}

QPainterPath PlacedItem::shape() const
{
    QPainterPath path;
    path.addRect(rect());

    if (!isSelected()) {
        return path;
    }

    const PlacedItemHandlePoints hp = placedItemHandlePoints(rect());
    const QPointF points[] = {
        hp.tl, hp.tm, hp.tr,
        hp.ml,        hp.mr,
        hp.bl, hp.bm, hp.br
    };

    for (const QPointF& point : points) {
        path.addEllipse(point, HANDLE_MARGIN_ITEM, HANDLE_MARGIN_ITEM);
    }

    return path;
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

    if ((m_verticalCenterGuide && m_verticalCenterGuide->parentItem() != monitor) ||
        (m_horizontalCenterGuide && m_horizontalCenterGuide->parentItem() != monitor)) {
        delete m_verticalCenterGuide;
        delete m_horizontalCenterGuide;
        m_verticalCenterGuide = nullptr;
        m_horizontalCenterGuide = nullptr;
    }

    QRectF monitorRect = monitor->rect();

    if (!m_verticalCenterGuide) {
        m_verticalCenterGuide = new QGraphicsLineItem(monitor);
        m_verticalCenterGuide->setZValue(9999);
    }

    if (!m_horizontalCenterGuide) {
        m_horizontalCenterGuide = new QGraphicsLineItem(monitor);
        m_horizontalCenterGuide->setZValue(9999);
    }

    m_verticalCenterGuide->setPen(QPen(AppTheme::palette().guide, 1.5, Qt::DashLine));
    m_horizontalCenterGuide->setPen(QPen(AppTheme::palette().guide, 1.5, Qt::DashLine));

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

void PlacedItem::updateDragGuides(MonitorDropItem* monitor)
{
    if (!monitor) {
        hideCenterGuides();
        return;
    }

    const QRectF monitorRect = monitor->rect();
    const QRectF draggedRect = monitor->mapRectFromScene(mapRectToScene(rect()));
    const QPointF monitorCenter = monitorRect.center();

    bool showVerticalGuide = false;
    bool showHorizontalGuide = false;
    qreal verticalGuideX = 0.0;
    qreal horizontalGuideY = 0.0;
    bool snapX = false;
    bool snapY = false;
    qreal bestDx = 0.0;
    qreal bestDy = 0.0;
    qreal bestAbsDx = CENTER_SNAP_PX + 1.0;
    qreal bestAbsDy = CENTER_SNAP_PX + 1.0;

    auto considerVertical = [&](qreal value, qreal target) {
        const qreal delta = target - value;
        const qreal distance = qAbs(delta);

        if (distance <= CENTER_SNAP_PX && distance < bestAbsDx) {
            snapX = true;
            bestDx = delta;
            bestAbsDx = distance;
            showVerticalGuide = true;
            verticalGuideX = target;
        }
    };

    auto considerHorizontal = [&](qreal value, qreal target) {
        const qreal delta = target - value;
        const qreal distance = qAbs(delta);

        if (distance <= CENTER_SNAP_PX && distance < bestAbsDy) {
            snapY = true;
            bestDy = delta;
            bestAbsDy = distance;
            showHorizontalGuide = true;
            horizontalGuideY = target;
        }
    };

    considerVertical(draggedRect.left(), monitorCenter.x());
    considerVertical(draggedRect.center().x(), monitorCenter.x());
    considerVertical(draggedRect.right(), monitorCenter.x());

    considerHorizontal(draggedRect.top(), monitorCenter.y());
    considerHorizontal(draggedRect.center().y(), monitorCenter.y());
    considerHorizontal(draggedRect.bottom(), monitorCenter.y());

    for (QGraphicsItem* child : monitor->childItems()) {
        const PlacedItem* other = dynamic_cast<const PlacedItem*>(child);

        if (!other || other == this) {
            continue;
        }

        const QRectF otherRect = monitor->mapRectFromItem(other, other->rect());

        const qreal draggedX[] = {
            draggedRect.left(),
            draggedRect.center().x(),
            draggedRect.right()
        };
        const qreal otherX[] = {
            otherRect.left(),
            otherRect.center().x(),
            otherRect.right()
        };

        for (qreal value : draggedX) {
            for (qreal target : otherX) {
                considerVertical(value, target);
            }
        }

        const qreal draggedY[] = {
            draggedRect.top(),
            draggedRect.center().y(),
            draggedRect.bottom()
        };
        const qreal otherY[] = {
            otherRect.top(),
            otherRect.center().y(),
            otherRect.bottom()
        };

        for (qreal value : draggedY) {
            for (qreal target : otherY) {
                considerHorizontal(value, target);
            }
        }
    }

    if (snapX || snapY) {
        QRectF snappedRect = draggedRect;
        snappedRect.translate(snapX ? bestDx : 0.0, snapY ? bestDy : 0.0);

        const QPointF sceneTopLeft = monitor->mapToScene(snappedRect.topLeft());
        const QPointF snappedItemPos = parentItem()
            ? parentItem()->mapFromScene(sceneTopLeft)
            : sceneTopLeft;

        if (!qFuzzyCompare(snappedItemPos.x(), pos().x()) ||
            !qFuzzyCompare(snappedItemPos.y(), pos().y())) {
            setPos(snappedItemPos);
        }
    }

    updateResizeGuides(
        monitor,
        showVerticalGuide,
        showHorizontalGuide,
        verticalGuideX,
        horizontalGuideY
    );
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
