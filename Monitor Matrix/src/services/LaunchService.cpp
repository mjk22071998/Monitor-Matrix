#include "LaunchService.h"

#include "LaunchProcess.h"
#include "LaunchWindowPlacement.h"

#include <QDebug>
#include <QStringList>

namespace {

QString chromePathFromRegistry()
{
    static constexpr const wchar_t* ChromeAppPath =
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe";

    const QString fromCurrentUser =
        LaunchProcess::registryDefaultStringValue(HKEY_CURRENT_USER, ChromeAppPath);

    if (!fromCurrentUser.isEmpty() && LaunchProcess::fileExists(fromCurrentUser))
        return fromCurrentUser;

    const QString fromMachine64 =
        LaunchProcess::registryDefaultStringValue(HKEY_LOCAL_MACHINE, ChromeAppPath, KEY_WOW64_64KEY);

    if (!fromMachine64.isEmpty() && LaunchProcess::fileExists(fromMachine64))
        return fromMachine64;

    const QString fromMachine32 =
        LaunchProcess::registryDefaultStringValue(HKEY_LOCAL_MACHINE, ChromeAppPath, KEY_WOW64_32KEY);

    if (!fromMachine32.isEmpty() && LaunchProcess::fileExists(fromMachine32))
        return fromMachine32;

    return QString();
}

QString findChromeWindowName()
{
    return QStringLiteral("chrome.exe");
}

} // namespace

LaunchService::LaunchService()
{
}

QString LaunchService::getChromePath()
{
    const QString registryPath = chromePathFromRegistry();

    if (!registryPath.isEmpty())
        return registryPath;

    const QString programFiles = LaunchProcess::environmentVariable(L"ProgramFiles");
    const QString programFilesX86 = LaunchProcess::environmentVariable(L"ProgramFiles(x86)");
    const QString localAppData = LaunchProcess::environmentVariable(L"LOCALAPPDATA");

    QStringList commonPaths;

    if (!programFiles.isEmpty()) {
        commonPaths.append(
            programFiles + "\\Google\\Chrome\\Application\\chrome.exe"
        );
    }

    if (!programFilesX86.isEmpty()) {
        commonPaths.append(
            programFilesX86 + "\\Google\\Chrome\\Application\\chrome.exe"
        );
    }

    if (!localAppData.isEmpty()) {
        commonPaths.append(
            localAppData + "\\Google\\Chrome\\Application\\chrome.exe"
        );
    }

    for (const QString& path : commonPaths) {
        if (LaunchProcess::fileExists(path))
            return path;
    }

    return QString();
}

LaunchService* LaunchService::getInstance()
{
    static LaunchService instance;
    return &instance;
}

bool LaunchService::launchExecutable(QString path, MonitorInfo info, AppPosition pos)
{
    return launchExecutable(path, info, pos, QRect());
}

bool LaunchService::launchExecutable(QString path,
    MonitorInfo info,
    AppPosition pos,
    const QRect& zoneRect)
{
    path = path.trimmed();

    const QString targetProcessName = LaunchProcess::fileNameFromPath(path);
    const QSet<HWND> windowsBefore = LaunchProcess::getTopLevelAppWindows();
    const QString workingDirectory = LaunchProcess::fileExists(path)
        ? LaunchProcess::directoryFromPath(path)
        : QString();

    const LaunchProcess::LaunchedProcess launched =
        LaunchProcess::startProcess(path, QStringList(), workingDirectory);

    if (!launched.started) {
        qWarning() << "Cannot start process" << targetProcessName
            << "error=" << GetLastError();
        return false;
    }

    HWND hwnd = LaunchProcess::waitForMainWindow(
        windowsBefore,
        launched.processId,
        targetProcessName
    );

    if (!hwnd)
        hwnd = LaunchProcess::findWindowByProcessId(launched.processId, nullptr, true);

    if (!hwnd)
        hwnd = LaunchProcess::findWindowByProcessName(targetProcessName, nullptr, true);

    if (!hwnd) {
        qWarning() << "Cannot find main window of" << targetProcessName;
        return false;
    }

    if (!LaunchWindowPlacement::placeWindow(hwnd, info, pos, zoneRect)) {
        qWarning() << "Cannot position window for" << targetProcessName;
        return false;
    }

    return true;
}

bool LaunchService::launchURL(QString url, MonitorInfo info, AppPosition pos)
{
    return launchURL(url, info, pos, QRect());
}

bool LaunchService::launchURL(QString url,
    MonitorInfo info,
    AppPosition pos,
    const QRect& zoneRect)
{
    const QString chromePath = getChromePath();

    if (chromePath.isEmpty()) {
        qWarning() << "Cannot find Chrome. Please install Google Chrome";
        return false;
    }

    const QSet<HWND> windowsBefore = LaunchProcess::getTopLevelAppWindows();
    const QStringList arguments = { "--new-window", url.trimmed() };

    const LaunchProcess::LaunchedProcess launched =
        LaunchProcess::startProcess(
            chromePath,
            arguments,
            LaunchProcess::directoryFromPath(chromePath)
        );

    if (!launched.started) {
        qWarning() << "Cannot start Google Chrome. error=" << GetLastError();
        return false;
    }

    const QString targetProcessName = findChromeWindowName();

    HWND hwnd = LaunchProcess::waitForMainWindow(
        windowsBefore,
        launched.processId,
        targetProcessName
    );

    if (!hwnd)
        hwnd = LaunchProcess::findWindowByProcessId(launched.processId, nullptr, true);

    if (!hwnd)
        hwnd = LaunchProcess::findWindowByProcessName(targetProcessName, nullptr, true);

    if (!hwnd) {
        qWarning() << "Cannot find main window for Google Chrome";
        return false;
    }

    if (!LaunchWindowPlacement::placeWindow(hwnd, info, pos, zoneRect)) {
        qWarning() << "Cannot position Google Chrome window";
        return false;
    }

    return true;
}
