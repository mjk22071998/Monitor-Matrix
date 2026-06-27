#include "LaunchProcess.h"

#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QtMath>

#include <string>
#include <vector>

namespace {

constexpr int MainWindowTimeoutMs = 10000;
constexpr int WindowPollIntervalMs = 80;

class ScopedHandle {
public:
    explicit ScopedHandle(HANDLE handle = nullptr)
        : m_handle(handle)
    {
    }

    ~ScopedHandle()
    {
        if (isValid())
            CloseHandle(m_handle);
    }

    ScopedHandle(const ScopedHandle&) = delete;
    ScopedHandle& operator=(const ScopedHandle&) = delete;

    bool isValid() const
    {
        return m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE;
    }

    HANDLE get() const
    {
        return m_handle;
    }

private:
    HANDLE m_handle = nullptr;
};

class ScopedRegKey {
public:
    explicit ScopedRegKey(HKEY key = nullptr)
        : m_key(key)
    {
    }

    ~ScopedRegKey()
    {
        if (m_key)
            RegCloseKey(m_key);
    }

    ScopedRegKey(const ScopedRegKey&) = delete;
    ScopedRegKey& operator=(const ScopedRegKey&) = delete;

    HKEY get() const
    {
        return m_key;
    }

private:
    HKEY m_key = nullptr;
};

QString expandEnvironmentStrings(const QString& value)
{
    DWORD length = ExpandEnvironmentStringsW(
        reinterpret_cast<LPCWSTR>(value.utf16()),
        nullptr,
        0
    );

    if (length == 0)
        return value;

    std::vector<wchar_t> buffer(length);
    length = ExpandEnvironmentStringsW(
        reinterpret_cast<LPCWSTR>(value.utf16()),
        buffer.data(),
        length
    );

    if (length == 0)
        return value;

    return QString::fromWCharArray(buffer.data());
}

QString quoteCommandLineArgument(const QString& argument)
{
    QString result = "\"";

    int backslashes = 0;

    for (QChar ch : argument) {
        if (ch == '\\') {
            ++backslashes;
            result += ch;
            continue;
        }

        if (ch == '"') {
            result += QString(backslashes + 1, '\\');
            result += ch;
            backslashes = 0;
            continue;
        }

        backslashes = 0;
        result += ch;
    }

    if (backslashes > 0)
        result += QString(backslashes, '\\');

    result += '"';
    return result;
}

QString commandLineFor(const QString& applicationPath,
    const QStringList& arguments)
{
    QString commandLine = quoteCommandLineArgument(applicationPath);

    for (const QString& argument : arguments) {
        commandLine += ' ';
        commandLine += quoteCommandLineArgument(argument);
    }

    return commandLine;
}

DWORD windowProcessId(HWND hwnd)
{
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

QString getProcessName(HWND hwnd)
{
    const DWORD pid = windowProcessId(hwnd);

    if (pid == 0)
        return QString();

    ScopedHandle process(
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)
    );

    if (!process.isValid())
        return QString();

    WCHAR path[MAX_PATH] = {};
    DWORD size = MAX_PATH;

    if (!QueryFullProcessImageNameW(process.get(), 0, path, &size))
        return QString();

    return LaunchProcess::fileNameFromPath(
        QString::fromWCharArray(path, static_cast<int>(size))
    );
}

bool isTopLevelAppWindow(HWND hwnd)
{
    if (!IsWindowVisible(hwnd))
        return false;

    if (GetWindow(hwnd, GW_OWNER) != nullptr)
        return false;

    const LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);

    return (style & WS_DISABLED) == 0;
}

QMutex& claimedWindowsMutex()
{
    static QMutex mutex;
    return mutex;
}

QSet<HWND>& claimedLaunchWindows()
{
    static QSet<HWND> windows;
    return windows;
}

void pruneClaimedLaunchWindows()
{
    QSet<HWND>& claimed = claimedLaunchWindows();

    for (auto it = claimed.begin(); it != claimed.end();) {
        if (!IsWindow(*it))
            it = claimed.erase(it);
        else
            ++it;
    }
}

}

namespace LaunchProcess {

QString fileNameFromPath(const QString& path)
{
    const int slash = qMax(path.lastIndexOf('\\'), path.lastIndexOf('/'));
    return slash >= 0 ? path.mid(slash + 1) : path;
}

QString directoryFromPath(const QString& path)
{
    const int slash = qMax(path.lastIndexOf('\\'), path.lastIndexOf('/'));
    return slash >= 0 ? path.left(slash) : QString();
}

bool fileExists(const QString& path)
{
    const DWORD attributes = GetFileAttributesW(
        reinterpret_cast<LPCWSTR>(path.utf16())
    );

    return attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

QString environmentVariable(const wchar_t* name)
{
    DWORD length = GetEnvironmentVariableW(name, nullptr, 0);

    if (length == 0)
        return QString();

    std::vector<wchar_t> buffer(length);
    length = GetEnvironmentVariableW(name, buffer.data(), length);

    if (length == 0)
        return QString();

    return QString::fromWCharArray(buffer.data(), static_cast<int>(length));
}

QString registryDefaultStringValue(HKEY root,
    const wchar_t* subkey,
    REGSAM viewFlags)
{
    HKEY rawKey = nullptr;

    const LONG openResult = RegOpenKeyExW(
        root,
        subkey,
        0,
        KEY_READ | viewFlags,
        &rawKey
    );

    if (openResult != ERROR_SUCCESS)
        return QString();

    ScopedRegKey key(rawKey);

    DWORD type = 0;
    DWORD size = 0;

    LONG queryResult = RegQueryValueExW(
        key.get(),
        nullptr,
        nullptr,
        &type,
        nullptr,
        &size
    );

    if (queryResult != ERROR_SUCCESS ||
        (type != REG_SZ && type != REG_EXPAND_SZ) ||
        size == 0) {
        return QString();
    }

    std::vector<wchar_t> buffer(size / sizeof(wchar_t) + 1);

    queryResult = RegQueryValueExW(
        key.get(),
        nullptr,
        nullptr,
        &type,
        reinterpret_cast<LPBYTE>(buffer.data()),
        &size
    );

    if (queryResult != ERROR_SUCCESS)
        return QString();

    QString value = QString::fromWCharArray(buffer.data()).trimmed();

    if (type == REG_EXPAND_SZ)
        value = expandEnvironmentStrings(value);

    return value;
}

LaunchedProcess startProcess(const QString& applicationPath,
    const QStringList& arguments,
    const QString& workingDirectory)
{
    LaunchedProcess launched;

    if (applicationPath.isEmpty())
        return launched;

    const QString commandLine =
        commandLineFor(applicationPath, arguments);

    std::wstring application = applicationPath.toStdWString();
    std::wstring command = commandLine.toStdWString();
    std::wstring workingDir = workingDirectory.toStdWString();

    STARTUPINFOW startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInfo = {};

    const BOOL ok = CreateProcessW(
        application.c_str(),
        command.data(),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        workingDirectory.isEmpty() ? nullptr : workingDir.c_str(),
        &startupInfo,
        &processInfo
    );

    if (!ok)
        return launched;

    launched.processId = processInfo.dwProcessId;
    launched.started = true;

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    return launched;
}

QSet<HWND> getTopLevelAppWindows()
{
    QSet<HWND> windows;

    EnumWindows([](HWND hwnd, LPARAM lparam) -> BOOL {
        if (isTopLevelAppWindow(hwnd))
            reinterpret_cast<QSet<HWND>*>(lparam)->insert(hwnd);

        return TRUE;
    }, reinterpret_cast<LPARAM>(&windows));

    return windows;
}

HWND findWindowByProcessId(DWORD processId,
    const QSet<HWND>* excludedWindows,
    bool claimWindow)
{
    if (processId == 0)
        return nullptr;

    const QSet<HWND> windows = getTopLevelAppWindows();
    QMutexLocker locker(&claimedWindowsMutex());
    pruneClaimedLaunchWindows();
    QSet<HWND>& claimed = claimedLaunchWindows();

    for (HWND hwnd : windows) {
        if (excludedWindows && excludedWindows->contains(hwnd))
            continue;

        if (claimed.contains(hwnd))
            continue;

        if (windowProcessId(hwnd) == processId) {
            if (claimWindow)
                claimed.insert(hwnd);

            return hwnd;
        }
    }

    return nullptr;
}

HWND findWindowByProcessName(const QString& targetProcessName,
    const QSet<HWND>* excludedWindows,
    bool claimWindow)
{
    if (targetProcessName.isEmpty())
        return nullptr;

    const QSet<HWND> windows = getTopLevelAppWindows();
    QMutexLocker locker(&claimedWindowsMutex());
    pruneClaimedLaunchWindows();
    QSet<HWND>& claimed = claimedLaunchWindows();

    for (HWND hwnd : windows) {
        if (excludedWindows && excludedWindows->contains(hwnd))
            continue;

        if (claimed.contains(hwnd))
            continue;

        const QString name = getProcessName(hwnd);

        if (name.compare(targetProcessName, Qt::CaseInsensitive) == 0) {
            if (claimWindow)
                claimed.insert(hwnd);

            return hwnd;
        }
    }

    return nullptr;
}

HWND waitForMainWindow(const QSet<HWND>& windowsBefore,
    DWORD launchedPid,
    const QString& targetProcessName)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < MainWindowTimeoutMs) {
        if (HWND hwnd = findWindowByProcessId(launchedPid, &windowsBefore, true))
            return hwnd;

        if (HWND hwnd = findWindowByProcessName(targetProcessName, &windowsBefore, true))
            return hwnd;

        if (HWND hwnd = findWindowByProcessId(launchedPid, nullptr, true))
            return hwnd;

        QThread::msleep(WindowPollIntervalMs);
    }

    return nullptr;
}

}
