#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <QSet>
#include <QString>
#include <QStringList>

namespace LaunchProcess {

struct LaunchedProcess {
    DWORD processId = 0;
    bool started = false;
};

QString fileNameFromPath(const QString& path);
QString directoryFromPath(const QString& path);
bool fileExists(const QString& path);
QString environmentVariable(const wchar_t* name);
QString registryDefaultStringValue(HKEY root,
    const wchar_t* subkey,
    REGSAM viewFlags = 0);

LaunchedProcess startProcess(const QString& applicationPath,
    const QStringList& arguments,
    const QString& workingDirectory);

QSet<HWND> getTopLevelAppWindows();
HWND findWindowByProcessId(DWORD processId,
    const QSet<HWND>* excludedWindows = nullptr,
    bool claimWindow = false);
HWND findWindowByProcessName(const QString& targetProcessName,
    const QSet<HWND>* excludedWindows = nullptr,
    bool claimWindow = false);
HWND waitForMainWindow(const QSet<HWND>& windowsBefore,
    DWORD launchedPid,
    const QString& targetProcessName);

}
