#include "PlacedItemResize.h"

#include "PlacedItemSnap.h"

QRectF resizedParentRectForHandle(
    const QRectF& originalRect,
    const QPointF& resizeStartParent,
    const QPointF& currentParent,
    PlacedItem::Handle handle
)
{
    const QPointF delta = currentParent - resizeStartParent;
    QRectF r = originalRect;

    switch (handle) {
    case PlacedItem::Handle::TopLeft:
        r.setTopLeft(r.topLeft() + delta);
        break;
    case PlacedItem::Handle::TopMid:
        r.setTop(r.top() + delta.y());
        break;
    case PlacedItem::Handle::TopRight:
        r.setTopRight(r.topRight() + delta);
        break;
    case PlacedItem::Handle::MidLeft:
        r.setLeft(r.left() + delta.x());
        break;
    case PlacedItem::Handle::MidRight:
        r.setRight(r.right() + delta.x());
        break;
    case PlacedItem::Handle::BotLeft:
        r.setBottomLeft(r.bottomLeft() + delta);
        break;
    case PlacedItem::Handle::BotMid:
        r.setBottom(r.bottom() + delta.y());
        break;
    case PlacedItem::Handle::BotRight:
        r.setBottomRight(r.bottomRight() + delta);
        break;
    default:
        break;
    }

    return r.normalized();
}

void applyMonitorCenterSnap(
    QRectF& r,
    const QRectF& monitorRect,
    PlacedItem::Handle handle,
    bool& showVerticalGuide,
    bool& showHorizontalGuide,
    qreal& verticalGuideX,
    qreal& horizontalGuideY
)
{
    const QPointF monitorCenter = monitorRect.center();

    const bool movesLeft =
        handle == PlacedItem::Handle::TopLeft ||
        handle == PlacedItem::Handle::MidLeft ||
        handle == PlacedItem::Handle::BotLeft;

    const bool movesRight =
        handle == PlacedItem::Handle::TopRight ||
        handle == PlacedItem::Handle::MidRight ||
        handle == PlacedItem::Handle::BotRight;

    const bool movesTop =
        handle == PlacedItem::Handle::TopLeft ||
        handle == PlacedItem::Handle::TopMid ||
        handle == PlacedItem::Handle::TopRight;

    const bool movesBottom =
        handle == PlacedItem::Handle::BotLeft ||
        handle == PlacedItem::Handle::BotMid ||
        handle == PlacedItem::Handle::BotRight;

    if (movesLeft) {
        qreal left = r.left();
        if (snapValue(left, monitorCenter.x())) {
            showVerticalGuide = true;
            verticalGuideX = monitorCenter.x();
        }
        r.setLeft(left);
    }

    if (movesRight) {
        qreal right = r.right();
        if (snapValue(right, monitorCenter.x())) {
            showVerticalGuide = true;
            verticalGuideX = monitorCenter.x();
        }
        r.setRight(right);
    }

    if (movesTop) {
        qreal top = r.top();
        if (snapValue(top, monitorCenter.y())) {
            showHorizontalGuide = true;
            horizontalGuideY = monitorCenter.y();
        }
        r.setTop(top);
    }

    if (movesBottom) {
        qreal bottom = r.bottom();
        if (snapValue(bottom, monitorCenter.y())) {
            showHorizontalGuide = true;
            horizontalGuideY = monitorCenter.y();
        }
        r.setBottom(bottom);
    }
}

QRectF clampRectToBounds(QRectF rect, const QRectF& bounds)
{
    rect = rect.normalized();

    if (rect.width() > bounds.width())
        rect.setWidth(bounds.width());

    if (rect.height() > bounds.height())
        rect.setHeight(bounds.height());

    if (rect.left() < bounds.left())
        rect.moveLeft(bounds.left());

    if (rect.top() < bounds.top())
        rect.moveTop(bounds.top());

    if (rect.right() > bounds.right())
        rect.moveRight(bounds.right());

    if (rect.bottom() > bounds.bottom())
        rect.moveBottom(bounds.bottom());

    return rect;
}

void enforceMinSizeForHandle(QRectF& r, PlacedItem::Handle handle)
{
    const bool movesLeft =
        handle == PlacedItem::Handle::TopLeft ||
        handle == PlacedItem::Handle::MidLeft ||
        handle == PlacedItem::Handle::BotLeft;

    const bool movesTop =
        handle == PlacedItem::Handle::TopLeft ||
        handle == PlacedItem::Handle::TopMid ||
        handle == PlacedItem::Handle::TopRight;

    if (r.width() < PlacedItem::MIN_SIZE) {
        if (movesLeft) {
            r.setLeft(r.right() - PlacedItem::MIN_SIZE);
        }
        else {
            r.setRight(r.left() + PlacedItem::MIN_SIZE);
        }
    }

    if (r.height() < PlacedItem::MIN_SIZE) {
        if (movesTop) {
            r.setTop(r.bottom() - PlacedItem::MIN_SIZE);
        }
        else {
            r.setBottom(r.top() + PlacedItem::MIN_SIZE);
        }
    }
}

void clampResizeRectToBounds(QRectF& r, const QRectF& bounds, PlacedItem::Handle handle)
{
    const bool movesLeft =
        handle == PlacedItem::Handle::TopLeft ||
        handle == PlacedItem::Handle::MidLeft ||
        handle == PlacedItem::Handle::BotLeft;

    const bool movesRight =
        handle == PlacedItem::Handle::TopRight ||
        handle == PlacedItem::Handle::MidRight ||
        handle == PlacedItem::Handle::BotRight;

    const bool movesTop =
        handle == PlacedItem::Handle::TopLeft ||
        handle == PlacedItem::Handle::TopMid ||
        handle == PlacedItem::Handle::TopRight;

    const bool movesBottom =
        handle == PlacedItem::Handle::BotLeft ||
        handle == PlacedItem::Handle::BotMid ||
        handle == PlacedItem::Handle::BotRight;

    if (movesLeft && r.left() < bounds.left()) {
        r.setLeft(bounds.left());
    }

    if (movesRight && r.right() > bounds.right()) {
        r.setRight(bounds.right());
    }

    if (movesTop && r.top() < bounds.top()) {
        r.setTop(bounds.top());
    }

    if (movesBottom && r.bottom() > bounds.bottom()) {
        r.setBottom(bounds.bottom());
    }

    enforceMinSizeForHandle(r, handle);
}
