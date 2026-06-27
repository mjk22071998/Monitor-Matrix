#include "LaunchWindowPlacement.h"

#include "../platform/windows/WindowsDisplayTopology.h"

#include <QDebug>
#include <QRectF>
#include <QtMath>

namespace {

QString rectToString(const QRect& rect)
{
    return QStringLiteral("x=%1 y=%2 w=%3 h=%4")
        .arg(rect.x())
        .arg(rect.y())
        .arg(rect.width())
        .arg(rect.height());
}

QString rectToString(const QRectF& rect)
{
    return QStringLiteral("x=%1 y=%2 w=%3 h=%4")
        .arg(rect.x(), 0, 'f', 2)
        .arg(rect.y(), 0, 'f', 2)
        .arg(rect.width(), 0, 'f', 2)
        .arg(rect.height(), 0, 'f', 2);
}

QRect calculateTargetRect(const QRect& monitorRect, AppPosition pos)
{
    const int width = monitorRect.width();
    const int height = monitorRect.height();

    QRect target = monitorRect;

    switch (pos) {
    case AppPosition::LEFT_HALF:
        target.setWidth(width / 2);
        break;
    case AppPosition::RIGHT_HALF:
        target.setX(monitorRect.x() + width / 2);
        target.setWidth(width - width / 2);
        break;
    case AppPosition::TOP_LEFT:
        target.setWidth(width / 2);
        target.setHeight(height / 2);
        break;
    case AppPosition::TOP_RIGHT:
        target.setX(monitorRect.x() + width / 2);
        target.setWidth(width - width / 2);
        target.setHeight(height / 2);
        break;
    case AppPosition::BOTTOM_LEFT:
        target.setY(monitorRect.y() + height / 2);
        target.setWidth(width / 2);
        target.setHeight(height - height / 2);
        break;
    case AppPosition::BOTTOM_RIGHT:
        target.setX(monitorRect.x() + width / 2);
        target.setY(monitorRect.y() + height / 2);
        target.setWidth(width - width / 2);
        target.setHeight(height - height / 2);
        break;
    case AppPosition::TOP_HALF:
        target.setHeight(height / 2);
        break;
    case AppPosition::BOTTOM_HALF:
        target.setY(monitorRect.y() + height / 2);
        target.setHeight(height - height / 2);
        break;
    case AppPosition::FULL_SCREEN:
    default:
        break;
    }

    return target;
}

QRect clampRectToBounds(QRect rect, const QRect& bounds)
{
    if (!bounds.isValid() || bounds.isEmpty())
        return rect;

    if (rect.width() > bounds.width())
        rect.setWidth(bounds.width());

    if (rect.height() > bounds.height())
        rect.setHeight(bounds.height());

    if (rect.x() < bounds.x())
        rect.moveLeft(bounds.x());

    if (rect.y() < bounds.y())
        rect.moveTop(bounds.y());

    const int boundsRight = bounds.x() + bounds.width();
    const int boundsBottom = bounds.y() + bounds.height();

    if (rect.x() + rect.width() > boundsRight)
        rect.moveLeft(boundsRight - rect.width());

    if (rect.y() + rect.height() > boundsBottom)
        rect.moveTop(boundsBottom - rect.height());

    return rect;
}

QRect mapZoneToNativeWorkArea(const WindowsNativeMonitor& native,
    const QRect& zoneRect)
{
    QRectF monitorRect = QRectF(native.monitorRect);

    if (!monitorRect.isValid() || monitorRect.isEmpty())
        return zoneRect;

    QRectF zone = QRectF(zoneRect);
    const QRectF localMonitor(0, 0, monitorRect.width(), monitorRect.height());

    if (!monitorRect.intersects(zone) && localMonitor.intersects(zone)) {
        qInfo().noquote()
            << "Custom zone looked monitor-local; translating to native monitor coordinates."
            << "device=" << native.deviceName
            << "zoneBefore=" << rectToString(zone)
            << "monitor=" << rectToString(monitorRect);

        zone.translate(monitorRect.topLeft());
    }

    const qreal xRatio = (zone.x() - monitorRect.x()) / monitorRect.width();
    const qreal yRatio = (zone.y() - monitorRect.y()) / monitorRect.height();
    const qreal widthRatio = zone.width() / monitorRect.width();
    const qreal heightRatio = zone.height() / monitorRect.height();

    QRect mapped(
        native.workRect.x() + qRound(xRatio * native.workRect.width()),
        native.workRect.y() + qRound(yRatio * native.workRect.height()),
        qRound(widthRatio * native.workRect.width()),
        qRound(heightRatio * native.workRect.height())
    );

    if (mapped.width() <= 0)
        mapped.setWidth(qMax(1, qRound(native.workRect.width() * 0.25)));

    if (mapped.height() <= 0)
        mapped.setHeight(qMax(1, qRound(native.workRect.height() * 0.25)));

    mapped = clampRectToBounds(mapped, native.workRect);

    qInfo().noquote()
        << "Mapped custom zone."
        << "device=" << native.deviceName
        << "monitor=" << rectToString(native.monitorRect)
        << "work=" << rectToString(native.workRect)
        << "zone=" << rectToString(zoneRect)
        << "target=" << rectToString(mapped);

    return mapped;
}

bool setWindowRect(HWND hwnd, const QRect& rect, UINT flags)
{
    return SetWindowPos(
        hwnd,
        nullptr,
        rect.x(),
        rect.y(),
        rect.width(),
        rect.height(),
        flags
    ) != FALSE;
}

void repaintWindow(HWND hwnd)
{
    RedrawWindow(
        hwnd,
        nullptr,
        nullptr,
        RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN
    );
}

}

namespace LaunchWindowPlacement {

bool placeWindow(HWND hwnd,
    const MonitorInfo& info,
    AppPosition pos,
    const QRect& zoneRect)
{
    if (!hwnd)
        return false;

    WindowsNativeMonitor native;
    QString resolveError;
    const bool hasNativeMonitor =
        WindowsDisplayTopology::resolveNativeMonitor(info, native, &resolveError);

    QRect targetArea = info.availableGeometry;

    if (hasNativeMonitor)
        targetArea = native.workRect;
    else if (!resolveError.isEmpty())
        qWarning().noquote() << resolveError;

    qInfo().noquote()
        << "Preparing window placement."
        << "device=" << info.deviceName
        << "monitor=" << rectToString(info.geometry)
        << "available=" << rectToString(info.availableGeometry)
        << "nativeMatched=" << hasNativeMonitor
        << "nativeMonitor=" << (hasNativeMonitor ? rectToString(native.monitorRect) : QString())
        << "nativeWork=" << (hasNativeMonitor ? rectToString(native.workRect) : QString())
        << "zone=" << (zoneRect.isNull() ? QStringLiteral("<none>") : rectToString(zoneRect));

    if (!zoneRect.isNull()) {
        const QRect targetRect = hasNativeMonitor
            ? mapZoneToNativeWorkArea(native, zoneRect)
            : zoneRect;

        ShowWindow(hwnd, SW_RESTORE);
        const bool moved = setWindowRect(
            hwnd,
            targetRect,
            SWP_NOZORDER | SWP_NOACTIVATE
        );

        repaintWindow(hwnd);
        return moved;
    }

    if (pos == AppPosition::FULL_SCREEN) {
        setWindowRect(
            hwnd,
            targetArea,
            SWP_NOZORDER | SWP_NOACTIVATE
        );

        ShowWindow(hwnd, SW_RESTORE);
        ShowWindow(hwnd, SW_MAXIMIZE);

        const bool moved = SetWindowPos(
            hwnd,
            HWND_TOP,
            targetArea.x(),
            targetArea.y(),
            targetArea.width(),
            targetArea.height(),
            SWP_SHOWWINDOW
        ) != FALSE;

        SetForegroundWindow(hwnd);
        repaintWindow(hwnd);
        return moved;
    }

    if (IsZoomed(hwnd))
        ShowWindow(hwnd, SW_RESTORE);

    const QRect targetRect = calculateTargetRect(targetArea, pos);
    const bool moved = setWindowRect(
        hwnd,
        targetRect,
        SWP_NOZORDER | SWP_NOACTIVATE
    );

    repaintWindow(hwnd);
    return moved;
}

}
