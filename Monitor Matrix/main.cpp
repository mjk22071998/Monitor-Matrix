#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCoreApplication>
#include <QDebug>

#include "src/controllers/ScreenDropEngine.h"
#include "src/controllers/DisplayConfigBridge.h"
#include "src/models/qml/AppCatalogModel.h"
#include "src/models/qml/MonitorCanvasModel.h"
#include "src/models/qml/PlacementModel.h"
#include "src/services/MonitorService.h"
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

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("ScreenDrop"));
    QCoreApplication::setApplicationName(QStringLiteral("Screen Drop"));

    AppLogger::initialize();

    qInfo().noquote() << "Log file:" << AppLogger::logFilePath();

    AppTheme::setMode(AppThemeMode::Dark);

    AppCatalogModel catalogModel;
    catalogModel.load();

    MonitorService monitorService;
    MonitorCanvasModel canvasModel(&monitorService);
    canvasModel.refreshMonitors();

    PlacementModel placementModel;
    placementModel.setMonitors(canvasModel.monitors());

    ScreenDropEngine screenDropEngine(&catalogModel, &canvasModel, &placementModel);
    DisplayConfigBridge displayConfigBridge;

    qmlRegisterSingletonInstance("ScreenDrop", 1, 0, "ScreenDropEngine", &screenDropEngine);
    qmlRegisterSingletonInstance("ScreenDrop", 1, 0, "DisplayConfigBridge", &displayConfigBridge);
    qmlRegisterSingletonInstance("ScreenDrop", 1, 0, "AppCatalogModel", &catalogModel);
    qmlRegisterSingletonInstance("ScreenDrop", 1, 0, "MonitorCanvasModel", &canvasModel);
    qmlRegisterSingletonInstance("ScreenDrop", 1, 0, "PlacementModel", &placementModel);

    QQmlApplicationEngine engine;

    QQmlContext* context = engine.rootContext();
    if (context) {
        context->setContextProperty(QStringLiteral("screenDropEngine"), &screenDropEngine);
        context->setContextProperty(QStringLiteral("displayConfigBridge"), &displayConfigBridge);
        context->setContextProperty(QStringLiteral("catalogModel"), &catalogModel);
        context->setContextProperty(QStringLiteral("canvasModel"), &canvasModel);
        context->setContextProperty(QStringLiteral("placementModel"), &placementModel);
    }

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection
    );

    engine.load(url);

    return app.exec();
}
