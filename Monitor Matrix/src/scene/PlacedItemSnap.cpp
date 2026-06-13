#include "PlacedItemSnap.h"

#include <qgraphicsitem.h>
#include <QtGlobal>

bool snapValue(qreal& value, qreal snapTarget)
{
    if (qAbs(value - snapTarget) <= PlacedItem::CENTER_SNAP_PX) {
        value = snapTarget;
        return true;
    }

    return false;
}

bool verticalRangesOverlap(const QRectF& a, const QRectF& b)
{
    return a.bottom() > b.top() && a.top() < b.bottom();
}

bool horizontalRangesOverlap(const QRectF& a, const QRectF& b)
{
    return a.right() > b.left() && a.left() < b.right();
}

void snapAndBlockResizeAgainstOtherItems(
    QRectF& r,
    const QRectF& originalRect,
    PlacedItem::Handle activeHandle,
    const PlacedItem* self,
    bool& showVerticalGuide,
    bool& showHorizontalGuide,
    qreal& verticalGuideX,
    qreal& horizontalGuideY
)
{
    QGraphicsItem* parent = self->parentItem();

    if (!parent) {
        return;
    }

    const bool movesLeft =
        activeHandle == PlacedItem::Handle::TopLeft ||
        activeHandle == PlacedItem::Handle::MidLeft ||
        activeHandle == PlacedItem::Handle::BotLeft;

    const bool movesRight =
        activeHandle == PlacedItem::Handle::TopRight ||
        activeHandle == PlacedItem::Handle::MidRight ||
        activeHandle == PlacedItem::Handle::BotRight;

    const bool movesTop =
        activeHandle == PlacedItem::Handle::TopLeft ||
        activeHandle == PlacedItem::Handle::TopMid ||
        activeHandle == PlacedItem::Handle::TopRight;

    const bool movesBottom =
        activeHandle == PlacedItem::Handle::BotLeft ||
        activeHandle == PlacedItem::Handle::BotMid ||
        activeHandle == PlacedItem::Handle::BotRight;

    for (QGraphicsItem* child : parent->childItems()) {
        const PlacedItem* other = dynamic_cast<const PlacedItem*>(child);

        if (!other || other == self) {
            continue;
        }

        QRectF otherRect = parent->mapRectFromItem(other, other->rect());

        // Pure vertical alignment snapping, even with no vertical overlap.
        if (movesLeft) {
            qreal left = r.left();

            if (snapValue(left, otherRect.left())) {
                r.setLeft(left);
                showVerticalGuide = true;
                verticalGuideX = otherRect.left();
            }
            else if (snapValue(left, otherRect.right())) {
                r.setLeft(left);
                showVerticalGuide = true;
                verticalGuideX = otherRect.right();
            }
        }

        if (movesRight) {
            qreal right = r.right();

            if (snapValue(right, otherRect.left())) {
                r.setRight(right);
                showVerticalGuide = true;
                verticalGuideX = otherRect.left();
            }
            else if (snapValue(right, otherRect.right())) {
                r.setRight(right);
                showVerticalGuide = true;
                verticalGuideX = otherRect.right();
            }
        }

        // Pure horizontal alignment snapping, even with no horizontal overlap.
        if (movesTop) {
            qreal top = r.top();

            if (snapValue(top, otherRect.top())) {
                r.setTop(top);
                showHorizontalGuide = true;
                horizontalGuideY = otherRect.top();
            }
            else if (snapValue(top, otherRect.bottom())) {
                r.setTop(top);
                showHorizontalGuide = true;
                horizontalGuideY = otherRect.bottom();
            }
        }

        if (movesBottom) {
            qreal bottom = r.bottom();

            if (snapValue(bottom, otherRect.top())) {
                r.setBottom(bottom);
                showHorizontalGuide = true;
                horizontalGuideY = otherRect.top();
            }
            else if (snapValue(bottom, otherRect.bottom())) {
                r.setBottom(bottom);
                showHorizontalGuide = true;
                horizontalGuideY = otherRect.bottom();
            }
        }

        // Hard overlap blocking only when the moving edge would actually collide.
        if (movesRight && originalRect.right() <= otherRect.left()) {
            if (verticalRangesOverlap(r, otherRect)) {
                if (r.right() > otherRect.left()) {
                    r.setRight(otherRect.left());
                    showVerticalGuide = true;
                    verticalGuideX = otherRect.left();
                }
            }
        }

        if (movesLeft && originalRect.left() >= otherRect.right()) {
            if (verticalRangesOverlap(r, otherRect)) {
                if (r.left() < otherRect.right()) {
                    r.setLeft(otherRect.right());
                    showVerticalGuide = true;
                    verticalGuideX = otherRect.right();
                }
            }
        }

        if (movesBottom && originalRect.bottom() <= otherRect.top()) {
            if (horizontalRangesOverlap(r, otherRect)) {
                if (r.bottom() > otherRect.top()) {
                    r.setBottom(otherRect.top());
                    showHorizontalGuide = true;
                    horizontalGuideY = otherRect.top();
                }
            }
        }

        if (movesTop && originalRect.top() >= otherRect.bottom()) {
            if (horizontalRangesOverlap(r, otherRect)) {
                if (r.top() < otherRect.bottom()) {
                    r.setTop(otherRect.bottom());
                    showHorizontalGuide = true;
                    horizontalGuideY = otherRect.bottom();
                }
            }
        }
    }
}

