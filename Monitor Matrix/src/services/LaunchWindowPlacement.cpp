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

bool setWindowRect(HWND hwnd, const QRect& rect, UINT flags)
{
    // Match the target window's DPI awareness context to prevent cross-process auto-scaling
    DPI_AWARENESS_CONTEXT oldContext = nullptr;
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        using GetWindowDpiAwarenessContextFn = DPI_AWARENESS_CONTEXT(WINAPI*)(HWND);
        using SetThreadDpiAwarenessContextFn = DPI_AWARENESS_CONTEXT(WINAPI*)(DPI_AWARENESS_CONTEXT);

        auto getContext = reinterpret_cast<GetWindowDpiAwarenessContextFn>(
            GetProcAddress(user32, "GetWindowDpiAwarenessContext"));
        auto setContext = reinterpret_cast<SetThreadDpiAwarenessContextFn>(
            GetProcAddress(user32, "SetThreadDpiAwarenessContext"));

        if (getContext && setContext) {
            DPI_AWARENESS_CONTEXT targetContext = getContext(hwnd);
            if (targetContext) {
                oldContext = setContext(targetContext);
            }
        }
    }

    const bool success = SetWindowPos(
        hwnd,
        nullptr,
        rect.x(),
        rect.y(),
        rect.width(),
        rect.height(),
        flags | SWP_FRAMECHANGED
    ) != FALSE;

    // Restore calling thread's original DPI awareness context
    if (oldContext && user32) {
        using SetThreadDpiAwarenessContextFn = DPI_AWARENESS_CONTEXT(WINAPI*)(DPI_AWARENESS_CONTEXT);
        auto setContext = reinterpret_cast<SetThreadDpiAwarenessContextFn>(
            GetProcAddress(user32, "SetThreadDpiAwarenessContext"));
        if (setContext) {
            setContext(oldContext);
        }
    }

    return success;
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

QRect mapZoneToNativeWorkArea(const WindowsNativeMonitor& native,
    const QRect& zoneRect)
{
    const QRect monitorRect = native.monitorRect;
    if (!monitorRect.isValid() || monitorRect.isEmpty())
        return zoneRect;

    const QRect workRect = (native.workRect.isValid() && !native.workRect.isEmpty())
        ? native.workRect
        : monitorRect;

    QRect zone = zoneRect;

    // Detect if zone is already in global virtual desktop coordinates.
    // A zone is considered global ONLY if it is completely inside monitorRect AND monitorRect is not at (0, 0).
    const bool isAlreadyGlobal = (monitorRect.topLeft() != QPoint(0, 0)) &&
        (zone.left() >= monitorRect.left() && zone.right() <= monitorRect.right() &&
         zone.top() >= monitorRect.top() && zone.bottom() <= monitorRect.bottom());

    qreal xOffset = 0.0;
    qreal yOffset = 0.0;

    if (isAlreadyGlobal) {
        xOffset = zone.x() - monitorRect.x();
        yOffset = zone.y() - monitorRect.y();
    } else {
        // zone is monitor-local (relative to (0, 0) of this monitor)
        xOffset = zone.x();
        yOffset = zone.y();
    }

    const qreal xRatio = xOffset / static_cast<qreal>(monitorRect.width());
    const qreal yRatio = yOffset / static_cast<qreal>(monitorRect.height());
    const qreal widthRatio = zone.width() / static_cast<qreal>(monitorRect.width());
    const qreal heightRatio = zone.height() / static_cast<qreal>(monitorRect.height());

    QRect mapped(
        workRect.x() + qRound(xRatio * workRect.width()),
        workRect.y() + qRound(yRatio * workRect.height()),
        qRound(widthRatio * workRect.width()),
        qRound(heightRatio * workRect.height())
    );

    if (mapped.width() <= 0)
        mapped.setWidth(qMax(1, qRound(workRect.width() * 0.25)));

    if (mapped.height() <= 0)
        mapped.setHeight(qMax(1, qRound(workRect.height() * 0.25)));

    mapped = clampRectToBounds(mapped, workRect);

    qInfo().noquote()
        << "Mapped custom zone."
        << "device=" << native.deviceName
        << "monitor=" << rectToString(native.monitorRect)
        << "work=" << rectToString(native.workRect)
        << "zone=" << rectToString(zoneRect)
        << "target=" << rectToString(mapped);

    return mapped;
}

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

        if (IsZoomed(hwnd) || IsIconic(hwnd)) {
            ShowWindow(hwnd, SW_RESTORE);
        }

        const bool moved = setWindowRect(
            hwnd,
            targetRect,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
        );

        // Verification pass: Moving windows across mixed-DPI monitors triggers WM_DPICHANGED in Windows.
        // If the target app adjusts its dimensions upon WM_DPICHANGED, re-applying guarantees placement adherence.
        RECT currentRect = {};
        if (GetWindowRect(hwnd, &currentRect)) {
            const int curW = currentRect.right - currentRect.left;
            const int curH = currentRect.bottom - currentRect.top;
            if (qAbs(currentRect.left - targetRect.x()) > 4 ||
                qAbs(currentRect.top - targetRect.y()) > 4 ||
                qAbs(curW - targetRect.width()) > 4 ||
                qAbs(curH - targetRect.height()) > 4) {
                setWindowRect(
                    hwnd,
                    targetRect,
                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
                );
            }
        }

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
