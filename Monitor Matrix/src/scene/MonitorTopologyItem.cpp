#include "MonitorTopologyItem.h"

#include <QBrush>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include "../theme/AppTheme.h"

MonitorTopologyItem::MonitorTopologyItem(int displayIndex,
    const QString& title,
    const QRectF& rect,
    QGraphicsItem* parent)
    : QGraphicsRectItem(rect, parent),
    m_displayIndex(displayIndex),
    m_title(title)
{
    setFlags(QGraphicsItem::ItemIsMovable |
        QGraphicsItem::ItemIsSelectable |
        QGraphicsItem::ItemSendsGeometryChanges);

    setAcceptHoverEvents(true);
}

int MonitorTopologyItem::displayIndex() const
{
    return m_displayIndex;
}

void MonitorTopologyItem::setPrimary(bool primary)
{
    m_primary = primary;
    update();
}

bool MonitorTopologyItem::isPrimary() const
{
    return m_primary;
}

void MonitorTopologyItem::setTitle(const QString& title)
{
    m_title = title;
    update();
}

QString MonitorTopologyItem::title() const
{
    return m_title;
}

QVariant MonitorTopologyItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    return QGraphicsRectItem::itemChange(change, value);
}

void MonitorTopologyItem::paint(QPainter* painter,
                                const QStyleOptionGraphicsItem* option,
                                QWidget* widget)
{
    Q_UNUSED(widget);

    const QRectF r = rect();
    const bool hovered = option->state & QStyle::State_MouseOver;
    const bool selected = isSelected();

    QColor border = hovered || selected
        ? AppTheme::palette().dockHoverBorder
        : AppTheme::palette().monitorBorder;

    QColor fill = hovered || selected
        ? AppTheme::palette().dockHoverFill
        : AppTheme::palette().dockFill;

    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(QPen(border, selected ? 2.8 : 1.8));
    painter->setBrush(fill);
    painter->drawRect(r);

    if (m_primary) {
        QPen primaryPen(AppTheme::palette().primaryDisplayBorder, 3.0);
        primaryPen.setStyle(Qt::DashLine);

        painter->setPen(primaryPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(r.adjusted(4, 4, -4, -4));
    }

    QFont f = painter->font();
    f.setBold(true);
    f.setPointSize(16);
    painter->setFont(f);

    painter->setPen(AppTheme::palette().placementText);
    painter->drawText(r, Qt::AlignCenter, m_title);

    if (m_primary) {
        QFont small = painter->font();
        small.setBold(true);
        small.setPointSize(9);
        painter->setFont(small);

        QRectF badgeRect(r.left() + 10, r.top() + 10, 76, 24);

        painter->setPen(Qt::NoPen);
        painter->setBrush(AppTheme::palette().primaryDisplayFill);
        painter->drawRect(badgeRect);

        painter->setPen(AppTheme::palette().primaryDisplayText);
        painter->drawText(badgeRect, Qt::AlignCenter, "PRIMARY");
    }
}
