#pragma once

#include <QGraphicsRectItem>
#include <QString>

class MonitorTopologyItem : public QGraphicsRectItem {
public:
    explicit MonitorTopologyItem(int displayIndex,
        const QString& title,
        const QRectF& rect,
        QGraphicsItem* parent = nullptr);

    int displayIndex() const;

    void setPrimary(bool primary);
    bool isPrimary() const;

    void setTitle(const QString& title);
    QString title() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void paint(QPainter* painter,
        const QStyleOptionGraphicsItem* option,
        QWidget* widget = nullptr) override;

private:
    int m_displayIndex = -1;
    QString m_title;
    bool m_primary = false;
};