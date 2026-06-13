#pragma once

#include <qglobal.h>
#include <qpoint.h>
#include <qrect.h>
#include <qtransform.h>

#include "PlacedItem.h"

struct PlacedItemHandlePoints {
    QPointF tl, tm, tr;
    QPointF ml, mr;
    QPointF bl, bm, br;
};

PlacedItemHandlePoints placedItemHandlePoints(const QRectF& rect);

qreal placedItemHandleRadiusFromTransform(
    const QTransform& itemToDevice,
    qreal pixels
);

PlacedItem::Handle placedItemHitTest(
    const QRectF& itemRect,
    const QPointF& itemPos,
    const QTransform& itemToDevice,
    qreal hitPixels
);

Qt::CursorShape cursorForPlacedItemHandle(PlacedItem::Handle handle);
