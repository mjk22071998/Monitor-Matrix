#include <qapplication.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qguiapplication.h>
#include "src/views/MainWindow.h"
#include "src/theme/AppTheme.h"
#include "src/utils/AppLogger.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

static void enablePerMonitorDpiAwareness()
{
    HMODULE user32 = GetModuleHandleW(L"user32.dll");

    if (user32) {
        using SetProcessDpiAwarenessContextFn =
            BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);

        auto setAwarenessContext =
            reinterpret_cast<SetProcessDpiAwarenessContextFn>(
                GetProcAddress(user32, "SetProcessDpiAwarenessContext")
            );

        if (setAwarenessContext) {
            if (setAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
                return;

            if (setAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE))
                return;
        }
    }

    SetProcessDPIAware();
}
#endif

int main(int argc, char** argv) {
#ifdef Q_OS_WIN
    enablePerMonitorDpiAwareness();
#endif

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough
    );

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("ScreenDrop"));
    QCoreApplication::setApplicationName(QStringLiteral("Screen Drop"));

    AppLogger::initialize();

    qInfo().noquote() << "Log file:" << AppLogger::logFilePath();

    AppTheme::apply(app, AppThemeMode::Dark);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
