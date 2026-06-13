#pragma once

#include <qglobal.h>
#include <qrect.h>

#include "PlacedItem.h"

bool snapValue(qreal& value, qreal snapTarget);
bool verticalRangesOverlap(const QRectF& a, const QRectF& b);
bool horizontalRangesOverlap(const QRectF& a, const QRectF& b);

void snapAndBlockResizeAgainstOtherItems(
    QRectF& r,
    const QRectF& originalRect,
    PlacedItem::Handle activeHandle,
    const PlacedItem* self,
    bool& showVerticalGuide,
    bool& showHorizontalGuide,
    qreal& verticalGuideX,
    qreal& horizontalGuideY
);
