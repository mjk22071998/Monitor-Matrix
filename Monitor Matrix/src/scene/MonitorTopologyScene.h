#pragma once

#include <QGraphicsScene>
#include <QHash>
#include <QPointF>
#include <QVector>
#include <QGraphicsLineItem>

#include "../models/DisplayConfigTypes.h"

class MonitorTopologyItem;

class MonitorTopologyScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit MonitorTopologyScene(QObject* parent = nullptr);

    void setDisplays(const QVector<DisplayConfigInfo>& displays);
    QVector<DisplayConfigInfo> displays() const;

    bool updateDisplaySize(int displayIndex, const QSize& size);
    void updatePrimaryDisplay(int displayIndex);

signals:
    void displaySelected(int displayIndex);
    void layoutChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    static constexpr qreal DISPLAY_SCALE = 1.0 / 8.0;
    static constexpr qreal REVERSE_DISPLAY_SCALE = 8.0;
    static constexpr qreal CONNECTION_TOLERANCE = 10.0;
    static constexpr qreal ALIGNMENT_SNAP_PX = 10.0;

    QVector<DisplayConfigInfo> m_displays;
    QVector<MonitorTopologyItem*> m_items;

    MonitorTopologyItem* m_dragItem = nullptr;
    QPointF m_lastValidDragPos;

    QGraphicsLineItem* m_verticalGuide = nullptr;
    QGraphicsLineItem* m_horizontalGuide = nullptr;

    void rebuildScene();
    void syncDisplaysFromItems();
    QRectF normalizedSceneRect() const;
    QRectF exactSceneRectFor(MonitorTopologyItem* item) const;

    MonitorTopologyItem* itemAtScenePos(const QPointF& scenePos) const;

    void rememberValidPosition(MonitorTopologyItem* item);
    void restoreLastValidPosition(MonitorTopologyItem* item);

    QPointF correctedPositionFor(MonitorTopologyItem* movingItem,
		const QPointF& releasedPos) const;

    QPointF snappedPositionFor(MonitorTopologyItem* movingItem,
        const QPointF& proposedPos) const;

    bool isCurrentLayoutValid() const;
    bool hasAnyOverlap() const;
    bool isConnectedLayout() const;

    bool rectsOverlap(const QRectF& a, const QRectF& b) const;
    bool rectsConnectedWithinTolerance(const QRectF& a, const QRectF& b) const;

    QPointF alignmentSnappedPositionFor(MonitorTopologyItem* movingItem,
        const QPointF& proposedPos,
        bool& showVerticalGuide,
        bool& showHorizontalGuide,
        qreal& verticalGuideX,
        qreal& horizontalGuideY) const;

    void updateAlignmentGuides(bool showVertical,
        bool showHorizontal,
        qreal verticalX,
        qreal horizontalY);

    void hideAlignmentGuides();
};