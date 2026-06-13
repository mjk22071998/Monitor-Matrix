#include "PlacedItemHandles.h"

#include <qline.h>

PlacedItemHandlePoints placedItemHandlePoints(const QRectF& rect)
{
    const qreal cx = rect.center().x();
    const qreal cy = rect.center().y();

    PlacedItemHandlePoints points;
    points.tl = rect.topLeft();
    points.tm = { cx, rect.top() };
    points.tr = rect.topRight();
    points.ml = { rect.left(), cy };
    points.mr = { rect.right(), cy };
    points.bl = rect.bottomLeft();
    points.bm = { cx, rect.bottom() };
    points.br = rect.bottomRight();

    return points;
}

qreal placedItemHandleRadiusFromTransform(const QTransform& itemToDevice, qreal pixels)
{
    bool ok = false;
    QTransform inv = itemToDevice.inverted(&ok);

    if (!ok) {
        return pixels;
    }

    QPointF a = inv.map(QPointF(0.0, 0.0));
    QPointF b = inv.map(QPointF(pixels, 0.0));

    return QLineF(a, b).length();
}

PlacedItem::Handle placedItemHitTest(
    const QRectF& itemRect,
    const QPointF& itemPos,
    const QTransform& itemToDevice,
    qreal hitPixels
)
{
    const qreal r = placedItemHandleRadiusFromTransform(itemToDevice, hitPixels);
    const PlacedItemHandlePoints hp = placedItemHandlePoints(itemRect);

    struct Entry {
        PlacedItem::Handle handle;
        QPointF point;
    };

    const Entry entries[] = {
        { PlacedItem::Handle::TopLeft, hp.tl },
        { PlacedItem::Handle::TopMid, hp.tm },
        { PlacedItem::Handle::TopRight, hp.tr },
        { PlacedItem::Handle::MidLeft, hp.ml },
        { PlacedItem::Handle::MidRight, hp.mr },
        { PlacedItem::Handle::BotLeft, hp.bl },
        { PlacedItem::Handle::BotMid, hp.bm },
        { PlacedItem::Handle::BotRight, hp.br }
    };

    for (const Entry& entry : entries) {
        if (QLineF(itemPos, entry.point).length() <= r) {
            return entry.handle;
        }
    }

    return PlacedItem::Handle::None;
}

Qt::CursorShape cursorForPlacedItemHandle(PlacedItem::Handle handle)
{
    switch (handle) {
    case PlacedItem::Handle::TopLeft:
    case PlacedItem::Handle::BotRight:
        return Qt::SizeFDiagCursor;

    case PlacedItem::Handle::TopRight:
    case PlacedItem::Handle::BotLeft:
        return Qt::SizeBDiagCursor;

    case PlacedItem::Handle::TopMid:
    case PlacedItem::Handle::BotMid:
        return Qt::SizeVerCursor;

    case PlacedItem::Handle::MidLeft:
    case PlacedItem::Handle::MidRight:
        return Qt::SizeHorCursor;

    default:
        return Qt::OpenHandCursor;
    }
}
