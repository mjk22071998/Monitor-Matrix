#include "MonitorTopologyScene.h"
#include "MonitorTopologyItem.h"

#include <QGraphicsSceneMouseEvent>
#include <QQueue>
#include <QtMath>

MonitorTopologyScene::MonitorTopologyScene(QObject* parent)
    : QGraphicsScene(parent)
{
}

void MonitorTopologyScene::setDisplays(const QVector<DisplayConfigInfo>& displays)
{
    m_displays = displays;
    rebuildScene();
}

QVector<DisplayConfigInfo> MonitorTopologyScene::displays() const
{
    return m_displays;
}

bool MonitorTopologyScene::updateDisplaySize(int displayIndex, const QSize& size)
{
    if (displayIndex < 0 || displayIndex >= m_items.size())
        return false;

    MonitorTopologyItem* resizedItem = m_items[displayIndex];

    const QRectF oldLocalRect = resizedItem->rect();
    const QRectF oldSceneRect = oldLocalRect.translated(resizedItem->pos());
    const QSize oldSize = m_displays[displayIndex].geometry.size();

    QVector<QPointF> oldPositions;
    oldPositions.reserve(m_items.size());

    for (MonitorTopologyItem* item : m_items)
        oldPositions.append(item->pos());

    QRectF newLocalRect = oldLocalRect;
    newLocalRect.setSize(QSizeF(
        size.width() * DISPLAY_SCALE,
        size.height() * DISPLAY_SCALE
    ));

    resizedItem->setRect(newLocalRect);
    m_displays[displayIndex].geometry.setSize(size);

    const QRectF newSceneRect = newLocalRect.translated(resizedItem->pos());

    const qreal dx = newSceneRect.right() - oldSceneRect.right();
    const qreal dy = newSceneRect.bottom() - oldSceneRect.bottom();

    auto verticalContact = [](const QRectF& a, const QRectF& b) {
        return qMin(a.bottom(), b.bottom()) - qMax(a.top(), b.top()) > 0.5;
        };

    auto horizontalContact = [](const QRectF& a, const QRectF& b) {
        return qMin(a.right(), b.right()) - qMax(a.left(), b.left()) > 0.5;
        };

    for (MonitorTopologyItem* item : m_items) {
        if (!item || item == resizedItem)
            continue;

        const QRectF otherRect = item->rect().translated(item->pos());

        const bool wasOnRight =
            qAbs(otherRect.left() - oldSceneRect.right()) <= 0.5 &&
            verticalContact(otherRect, oldSceneRect);

        const bool wasBelow =
            qAbs(otherRect.top() - oldSceneRect.bottom()) <= 0.5 &&
            horizontalContact(otherRect, oldSceneRect);

        QPointF pos = item->pos();

        if (wasOnRight)
            pos.rx() += dx;

        if (wasBelow)
            pos.ry() += dy;

        item->setPos(pos);
    }

    if (!isCurrentLayoutValid()) {
        resizedItem->setRect(oldLocalRect);
        m_displays[displayIndex].geometry.setSize(oldSize);

        for (int i = 0; i < m_items.size() && i < oldPositions.size(); ++i)
            m_items[i]->setPos(oldPositions[i]);

        return false;
    }

    rememberValidPosition(resizedItem);
    syncDisplaysFromItems();

    emit layoutChanged();
    return true;
}
void MonitorTopologyScene::updatePrimaryDisplay(int displayIndex)
{
    if (displayIndex < 0 || displayIndex >= m_displays.size())
        return;

    for (int i = 0; i < m_displays.size(); ++i) {
        m_displays[i].isPrimary = (i == displayIndex);

        if (i < m_items.size())
            m_items[i]->setPrimary(i == displayIndex);
    }

    emit layoutChanged();
}

void MonitorTopologyScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    m_dragItem = itemAtScenePos(event->scenePos());

    if (m_dragItem) {
        m_lastValidDragPos = m_dragItem->pos();
        emit displaySelected(m_dragItem->displayIndex());
    }

    QGraphicsScene::mousePressEvent(event);
}

void MonitorTopologyScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseMoveEvent(event);

    if (!m_dragItem)
        return;

    bool showVerticalGuide = false;
    bool showHorizontalGuide = false;
    qreal verticalGuideX = 0.0;
    qreal horizontalGuideY = 0.0;

    const QPointF snappedPos = alignmentSnappedPositionFor(
        m_dragItem,
        m_dragItem->pos(),
        showVerticalGuide,
        showHorizontalGuide,
        verticalGuideX,
        horizontalGuideY
    );

    m_dragItem->setPos(snappedPos);

    updateAlignmentGuides(
        showVerticalGuide,
        showHorizontalGuide,
        verticalGuideX,
        horizontalGuideY
    );
}

void MonitorTopologyScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);

    if (!m_dragItem)
        return;

    hideAlignmentGuides();

    const QPointF releasedPos = m_dragItem->pos();

    QPointF correctedPos = snappedPositionFor(m_dragItem, releasedPos);
    m_dragItem->setPos(correctedPos);

    if (!isCurrentLayoutValid()) {
        correctedPos = correctedPositionFor(m_dragItem, releasedPos);
        m_dragItem->setPos(correctedPos);
    }

    if (!isCurrentLayoutValid()) {
        restoreLastValidPosition(m_dragItem);
    }
    else {
        rememberValidPosition(m_dragItem);
        syncDisplaysFromItems();
        emit layoutChanged();
    }

    m_dragItem = nullptr;
}

void MonitorTopologyScene::rebuildScene()
{
    clear();

    m_items.clear();
    m_dragItem = nullptr;
    m_verticalGuide = nullptr;
    m_horizontalGuide = nullptr;

    for (int i = 0; i < m_displays.size(); ++i) {
        const QRect g = m_displays[i].geometry;

        QRectF sceneRect(
            g.x() * DISPLAY_SCALE,
            g.y() * DISPLAY_SCALE,
            g.width() * DISPLAY_SCALE,
            g.height() * DISPLAY_SCALE
        );

        auto* item = new MonitorTopologyItem(
            i,
            QString::number(i + 1),
            QRectF(0, 0, sceneRect.width(), sceneRect.height())
        );

        item->setPos(sceneRect.topLeft());
        item->setPrimary(m_displays[i].isPrimary);

        addItem(item);
        m_items.append(item);

        rememberValidPosition(item);
    }

    setSceneRect(normalizedSceneRect().adjusted(-80, -80, 80, 80));
}

void MonitorTopologyScene::syncDisplaysFromItems()
{
    for (int i = 0; i < m_items.size() && i < m_displays.size(); ++i) {
        MonitorTopologyItem* item = m_items[i];

        const QPointF p = item->pos();

        QRect newGeometry(
            qRound(p.x() * REVERSE_DISPLAY_SCALE),
            qRound(p.y() * REVERSE_DISPLAY_SCALE),
            m_displays[i].geometry.width(),
            m_displays[i].geometry.height()
        );

        m_displays[i].geometry = newGeometry;
    }

    setSceneRect(normalizedSceneRect().adjusted(-80, -80, 80, 80));
}

QRectF MonitorTopologyScene::normalizedSceneRect() const
{
    QRectF result;

    for (MonitorTopologyItem* item : m_items) {
        const QRectF itemSceneRect = exactSceneRectFor(item);

        if (result.isNull())
            result = itemSceneRect;
        else
            result = result.united(itemSceneRect);
    }

    if (result.isNull())
        result = QRectF(0, 0, 800, 400);

    return result;
}

MonitorTopologyItem* MonitorTopologyScene::itemAtScenePos(const QPointF& scenePos) const
{
    const QList<QGraphicsItem*> hitItems = items(scenePos);

    for (QGraphicsItem* hit : hitItems) {
        auto* item = dynamic_cast<MonitorTopologyItem*>(hit);
        if (item)
            return item;
    }

    return nullptr;
}

void MonitorTopologyScene::rememberValidPosition(MonitorTopologyItem* item)
{
    if (!item)
        return;

    m_lastValidDragPos = item->pos();
}

void MonitorTopologyScene::restoreLastValidPosition(MonitorTopologyItem* item)
{
    if (!item)
        return;

    item->setPos(m_lastValidDragPos);
}

QPointF MonitorTopologyScene::snappedPositionFor(MonitorTopologyItem* movingItem,
    const QPointF& proposedPos) const
{
    if (!movingItem)
        return proposedPos;

    QRectF movingRect = movingItem->rect().translated(proposedPos);

    qreal bestDx = 0.0;
    qreal bestDy = 0.0;
    qreal bestAbsDx = CONNECTION_TOLERANCE + 1.0;
    qreal bestAbsDy = CONNECTION_TOLERANCE + 1.0;

    for (MonitorTopologyItem* other : m_items) {
        if (!other || other == movingItem)
            continue;

        const QRectF otherRect = other->sceneBoundingRect();

        const qreal dxCandidates[] = {
            otherRect.left() - movingRect.left(),
            otherRect.left() - movingRect.right(),
            otherRect.right() - movingRect.left(),
            otherRect.right() - movingRect.right()
        };

        for (qreal dx : dxCandidates) {
            if (qAbs(dx) < bestAbsDx) {
                QRectF testRect = movingRect.translated(dx, 0);

                const bool verticallyRelevant =
                    testRect.bottom() >= otherRect.top() - CONNECTION_TOLERANCE &&
                    testRect.top() <= otherRect.bottom() + CONNECTION_TOLERANCE;

                if (verticallyRelevant) {
                    bestAbsDx = qAbs(dx);
                    bestDx = dx;
                }
            }
        }

        const qreal dyCandidates[] = {
            otherRect.top() - movingRect.top(),
            otherRect.top() - movingRect.bottom(),
            otherRect.bottom() - movingRect.top(),
            otherRect.bottom() - movingRect.bottom()
        };

        for (qreal dy : dyCandidates) {
            if (qAbs(dy) < bestAbsDy) {
                QRectF testRect = movingRect.translated(0, dy);

                const bool horizontallyRelevant =
                    testRect.right() >= otherRect.left() - CONNECTION_TOLERANCE &&
                    testRect.left() <= otherRect.right() + CONNECTION_TOLERANCE;

                if (horizontallyRelevant) {
                    bestAbsDy = qAbs(dy);
                    bestDy = dy;
                }
            }
        }
    }

    QPointF snappedPos = proposedPos;

    if (bestAbsDx <= CONNECTION_TOLERANCE)
        snappedPos.rx() += bestDx;

    if (bestAbsDy <= CONNECTION_TOLERANCE)
        snappedPos.ry() += bestDy;

    return snappedPos;
}

QPointF MonitorTopologyScene::correctedPositionFor(MonitorTopologyItem* movingItem,
    const QPointF& releasedPos) const
{
    if (!movingItem)
        return releasedPos;

    const QRectF movingLocalRect = movingItem->rect();
    const QRectF releasedRect = movingLocalRect.translated(releasedPos);
    const QPointF releasedCenter = releasedRect.center();

    MonitorTopologyItem* nearestItem = nullptr;
    qreal nearestDistance = std::numeric_limits<qreal>::max();

    for (MonitorTopologyItem* other : m_items) {
        if (!other || other == movingItem)
            continue;

        const QRectF otherRect = other->sceneBoundingRect();
        const qreal distance = QLineF(releasedCenter, otherRect.center()).length();

        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearestItem = other;
        }
    }

    if (!nearestItem)
        return releasedPos;

    const QRectF otherRect = nearestItem->sceneBoundingRect();

    const qreal movingW = movingLocalRect.width();
    const qreal movingH = movingLocalRect.height();
    const qreal requiredContact = CONNECTION_TOLERANCE;

    QVector<QPointF> candidates;

    auto clamp = [](qreal value, qreal minValue, qreal maxValue) -> qreal {
        if (minValue > maxValue)
            return minValue;

        return qMax(minValue, qMin(value, maxValue));
        };

    const qreal preferredX = releasedPos.x();
    const qreal preferredY = releasedPos.y();

    // Place moving screen to the RIGHT of nearest screen.
    // Force at least 15px vertical side contact.
    {
        const qreal x = otherRect.right();
        const qreal y = clamp(
            preferredY,
            otherRect.top() - movingH + requiredContact,
            otherRect.bottom() - requiredContact
        );

        candidates.append(QPointF(x, y));
    }

    // Place moving screen to the LEFT of nearest screen.
    // Force at least 15px vertical side contact.
    {
        const qreal x = otherRect.left() - movingW;
        const qreal y = clamp(
            preferredY,
            otherRect.top() - movingH + requiredContact,
            otherRect.bottom() - requiredContact
        );

        candidates.append(QPointF(x, y));
    }

    // Place moving screen BELOW nearest screen.
    // Force at least 15px horizontal side contact.
    {
        const qreal x = clamp(
            preferredX,
            otherRect.left() - movingW + requiredContact,
            otherRect.right() - requiredContact
        );
        const qreal y = otherRect.bottom();

        candidates.append(QPointF(x, y));
    }

    // Place moving screen ABOVE nearest screen.
    // Force at least 15px horizontal side contact.
    {
        const qreal x = clamp(
            preferredX,
            otherRect.left() - movingW + requiredContact,
            otherRect.right() - requiredContact
        );
        const qreal y = otherRect.top() - movingH;

        candidates.append(QPointF(x, y));
    }

    QPointF bestPos = releasedPos;
    qreal bestScore = std::numeric_limits<qreal>::max();

    const QPointF oldPos = movingItem->pos();

    for (const QPointF& candidate : candidates) {
        movingItem->setPos(candidate);

        if (!isCurrentLayoutValid())
            continue;

        const qreal score = QLineF(candidate, releasedPos).length();

        if (score < bestScore) {
            bestScore = score;
            bestPos = candidate;
        }
    }

    movingItem->setPos(oldPos);

    return bestPos;
}

bool MonitorTopologyScene::isCurrentLayoutValid() const
{
    if (m_items.size() <= 1)
        return true;

    return !hasAnyOverlap() && isConnectedLayout();
}

bool MonitorTopologyScene::hasAnyOverlap() const
{
    for (int i = 0; i < m_items.size(); ++i) {
        for (int j = i + 1; j < m_items.size(); ++j) {
			if (rectsOverlap(exactSceneRectFor(m_items[i]),
				exactSceneRectFor(m_items[j]))) {
                return true;
            }
        }
    }

    return false;
}

bool MonitorTopologyScene::isConnectedLayout() const
{
    if (m_items.size() <= 1)
        return true;

    QVector<bool> visited(m_items.size(), false);
    QQueue<int> queue;

    visited[0] = true;
    queue.enqueue(0);

    while (!queue.isEmpty()) {
        const int current = queue.dequeue();
        const QRectF currentRect = exactSceneRectFor(m_items[current]);

        for (int i = 0; i < m_items.size(); ++i) {
            if (visited[i] || i == current)
                continue;

            const QRectF otherRect = exactSceneRectFor(m_items[i]);

            if (rectsConnectedWithinTolerance(currentRect, otherRect)) {
                visited[i] = true;
                queue.enqueue(i);
            }
        }
    }

    for (bool v : visited) {
        if (!v)
            return false;
    }

    return true;
}

bool MonitorTopologyScene::rectsOverlap(const QRectF& a, const QRectF& b) const
{
    const QRectF intersection = a.intersected(b);

    return intersection.width() > 0.5 && intersection.height() > 0.5;
}

bool MonitorTopologyScene::rectsConnectedWithinTolerance(const QRectF& a,
    const QRectF& b) const
{
    const qreal requiredContact = CONNECTION_TOLERANCE;

    const bool horizontalSideTouch =
        qAbs(a.right() - b.left()) <= 0.5 ||
        qAbs(b.right() - a.left()) <= 0.5;

    if (horizontalSideTouch) {
        const qreal overlapTop = qMax(a.top(), b.top());
        const qreal overlapBottom = qMin(a.bottom(), b.bottom());
        const qreal contactLength = overlapBottom - overlapTop;

        if (contactLength >= requiredContact)
            return true;
    }

    const bool verticalSideTouch =
        qAbs(a.bottom() - b.top()) <= 0.5 ||
        qAbs(b.bottom() - a.top()) <= 0.5;

    if (verticalSideTouch) {
        const qreal overlapLeft = qMax(a.left(), b.left());
        const qreal overlapRight = qMin(a.right(), b.right());
        const qreal contactLength = overlapRight - overlapLeft;

        if (contactLength >= requiredContact)
            return true;
    }

    return false;
}

QPointF MonitorTopologyScene::alignmentSnappedPositionFor(
    MonitorTopologyItem* movingItem,
    const QPointF& proposedPos,
    bool& showVerticalGuide,
    bool& showHorizontalGuide,
    qreal& verticalGuideX,
    qreal& horizontalGuideY) const
{
    if (!movingItem)
        return proposedPos;

    QRectF movingRect = movingItem->rect().translated(proposedPos);

    QPointF result = proposedPos;

    qreal bestDx = 0.0;
    qreal bestDy = 0.0;
    qreal bestAbsDx = ALIGNMENT_SNAP_PX + 1.0;
    qreal bestAbsDy = ALIGNMENT_SNAP_PX + 1.0;

    for (MonitorTopologyItem* other : m_items) {
        if (!other || other == movingItem)
            continue;

        const QRectF otherRect = other->sceneBoundingRect();

        const qreal movingXs[] = {
            movingRect.left(),
            movingRect.right()
        };

        const qreal otherXs[] = {
            otherRect.left(),
            otherRect.right()
        };

        for (qreal movingX : movingXs) {
            for (qreal otherX : otherXs) {
                const qreal dx = otherX - movingX;

                if (qAbs(dx) <= ALIGNMENT_SNAP_PX &&
                    qAbs(dx) < bestAbsDx) {
                    bestAbsDx = qAbs(dx);
                    bestDx = dx;
                    verticalGuideX = otherX;
                    showVerticalGuide = true;
                }
            }
        }

        const qreal movingYs[] = {
            movingRect.top(),
            movingRect.bottom()
        };

        const qreal otherYs[] = {
            otherRect.top(),
            otherRect.bottom()
        };

        for (qreal movingY : movingYs) {
            for (qreal otherY : otherYs) {
                const qreal dy = otherY - movingY;

                if (qAbs(dy) <= ALIGNMENT_SNAP_PX &&
                    qAbs(dy) < bestAbsDy) {
                    bestAbsDy = qAbs(dy);
                    bestDy = dy;
                    horizontalGuideY = otherY;
                    showHorizontalGuide = true;
                }
            }
        }
    }

    if (showVerticalGuide)
        result.rx() += bestDx;

    if (showHorizontalGuide)
        result.ry() += bestDy;

    return result;
}

void MonitorTopologyScene::updateAlignmentGuides(bool showVertical,
    bool showHorizontal,
    qreal verticalX,
    qreal horizontalY)
{
    QRectF guideBounds = normalizedSceneRect().adjusted(-120, -120, 120, 120);

    if (!m_verticalGuide) {
        m_verticalGuide = new QGraphicsLineItem();
        m_verticalGuide->setPen(QPen(QColor(32, 210, 180, 220), 1.5, Qt::DashLine));
        m_verticalGuide->setZValue(99999);
        addItem(m_verticalGuide);
    }

    if (!m_horizontalGuide) {
        m_horizontalGuide = new QGraphicsLineItem();
        m_horizontalGuide->setPen(QPen(QColor(32, 210, 180, 220), 1.5, Qt::DashLine));
        m_horizontalGuide->setZValue(99999);
        addItem(m_horizontalGuide);
    }

    m_verticalGuide->setLine(
        verticalX,
        guideBounds.top(),
        verticalX,
        guideBounds.bottom()
    );

    m_horizontalGuide->setLine(
        guideBounds.left(),
        horizontalY,
        guideBounds.right(),
        horizontalY
    );

    m_verticalGuide->setVisible(showVertical);
    m_horizontalGuide->setVisible(showHorizontal);
}

void MonitorTopologyScene::hideAlignmentGuides()
{
    if (m_verticalGuide)
        m_verticalGuide->setVisible(false);

    if (m_horizontalGuide)
        m_horizontalGuide->setVisible(false);
}

QRectF MonitorTopologyScene::exactSceneRectFor(MonitorTopologyItem* item) const
{
    if (!item)
        return QRectF();

    return item->rect().translated(item->pos());
}