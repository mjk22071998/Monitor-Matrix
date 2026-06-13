#pragma once

#include <qpoint.h>
#include <qrect.h>

#include "PlacedItem.h"

QRectF resizedParentRectForHandle(
    const QRectF& originalRect,
    const QPointF& resizeStartParent,
    const QPointF& currentParent,
    PlacedItem::Handle handle
);

void applyMonitorCenterSnap(
    QRectF& r,
    const QRectF& monitorRect,
    PlacedItem::Handle handle,
    bool& showVerticalGuide,
    bool& showHorizontalGuide,
    qreal& verticalGuideX,
    qreal& horizontalGuideY
);

QRectF clampRectToBounds(QRectF rect, const QRectF& bounds);

void enforceMinSizeForHandle(QRectF& r, PlacedItem::Handle handle);

void clampResizeRectToBounds(
    QRectF& r,
    const QRectF& bounds,
    PlacedItem::Handle handle
);
