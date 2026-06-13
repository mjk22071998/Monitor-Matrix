#pragma once

#include <qobject.h>
#include <qgraphicsitem.h>
#include <qvariant.h>

#include "../enums/AppPositionEnum.h"

class MonitorDropItem;

class PlacedItem : public QObject, public QGraphicsRectItem {
    Q_OBJECT

public:
    enum class Handle {
        None,
        TopLeft, TopMid, TopRight,
        MidLeft, MidRight,
        BotLeft, BotMid, BotRight
    };

    static constexpr qreal CENTER_SNAP_PX = 10.0;
    static constexpr qreal MIN_SIZE = 40.0;

    explicit PlacedItem(
        const QString& placementId,
        const QString& appId,
        const QString& appName,
        const QRectF& zoneRect,
        QGraphicsItem* parent = nullptr
    );

    QString getPlacementId() const;
    QString getAppId() const;
    QString getAppName() const;

    void setHasOverlap(bool hasOverlap);
    bool hasOverlap() const;

    QRectF boundingRect() const override;

signals:
    void moveRequested(
        const QString& placementId,
        const QString& appId,
        const QString& appName,
        const QString& targetDeviceName,
        AppPosition targetZone
    );

    void customRectRequested(
        const QString& placementId,
        const QString& appId,
        const QString& appName,
        const QString& targetDeviceName,
        const QRectF& zoneRect
    );

    void removeRequested(const QString& placementId);

protected:
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent*) override;
    QVariant itemChange(GraphicsItemChange, const QVariant&) override;

private:
    QString m_placementId;
    QString m_appId;
    QString m_appName;

    bool m_hasOverlap = false;

    QGraphicsLineItem* m_verticalCenterGuide = nullptr;
    QGraphicsLineItem* m_horizontalCenterGuide = nullptr;

    QGraphicsTextItem* m_label = nullptr;
    MonitorDropItem* p_lastHighlightedMonitor = nullptr;

    Handle m_activeHandle = Handle::None;
    bool m_resizing = false;
    bool m_hasMoved = false;

    QRectF m_origParentRect;
    QPointF m_resizeStartParent;

    static constexpr qreal HANDLE_PX = 4.5;
    static constexpr qreal HANDLE_HIT_PX = 9.0;
    static constexpr qreal HANDLE_MARGIN_ITEM = 80.0;

    void clearDragHighlight();

    void updateResizeGuides(
        MonitorDropItem* monitor,
        bool showVertical,
        bool showHorizontal,
        qreal verticalX,
        qreal horizontalY
    );

    void hideCenterGuides();
};