#include "LaunchService.h"
#include <Windows.h>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qfile.h>
#include <qdir.h>
#include <qprocess.h>
#include <qthread.h>
#include <shellapi.h>
#include <TlHelp32.h>
#include <vector>
#include <string>

using std::vector;

LaunchService* LaunchService::p_instance = nullptr;

static RECT calculateTargetRect(const RECT& monitorRect, AppPosition pos) {
	LONG width = monitorRect.right - monitorRect.left;
	LONG height = monitorRect.bottom - monitorRect.top;

	RECT target = monitorRect;

	switch (pos) {
	case AppPosition::LEFT_HALF:
		target.right = monitorRect.left + width / 2;
		break;
	case AppPosition::RIGHT_HALF:
		target.left = monitorRect.left + width / 2;
		break;
	case AppPosition::TOP_LEFT:
		target.right = monitorRect.left + width / 2;
		target.bottom = monitorRect.top + height / 2;
		break;
	case AppPosition::TOP_RIGHT:
		target.left = monitorRect.left + width / 2;
		target.bottom = monitorRect.top + height / 2;
		break;
	case AppPosition::BOTTOM_LEFT:
		target.right = monitorRect.left + width / 2;
		target.top = monitorRect.top + height / 2;
		break;
	case AppPosition::BOTTOM_RIGHT:
		target.left = monitorRect.left + width / 2;
		target.top = monitorRect.top + height / 2;
		break;
	case AppPosition::TOP_HALF:
		target.bottom = monitorRect.top + height / 2;
		break;
	case AppPosition::BOTTOM_HALF:
		target.top = monitorRect.top + height / 2;
		break;
	case AppPosition::FULL_SCREEN:
	default:
		break;
	}
	return target;
}

static QString getProcessName(HWND hwnd) {
	DWORD pid = 0;

	GetWindowThreadProcessId(hwnd, &pid);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (!hProcess) 
		return QString();
	
	WCHAR path[MAX_PATH];
	DWORD size = MAX_PATH;
	if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
		CloseHandle(hProcess);
		return QFileInfo(QString::fromWCharArray(path)).fileName();
	}

	CloseHandle(hProcess);
	return QString();
}

static QSet<HWND> getAllVisibleWindows() {
	QSet<HWND> windows;
	EnumWindows([](HWND hwnd, LPARAM lparam)->BOOL {
		if (IsWindowVisible(hwnd)) {
			reinterpret_cast<QSet<HWND>*>(lparam)->insert(hwnd);
		}
		return TRUE;
		}, reinterpret_cast<LPARAM>(&windows));
	return windows;
}

static HWND waitForMainWindow(QSet<HWND> &windowsBefore, const QString &targetProcessName, int timeoutMs=10000) {
	DWORD start = GetTickCount64();

	while (GetTickCount64() - start < static_cast<DWORD>(timeoutMs)) {
		QSet<HWND> windowsNow = getAllVisibleWindows();
		QSet<HWND> newWindows = windowsNow - windowsBefore;
		for (HWND hwnd : newWindows) {
			if (getProcessName(hwnd).compare(targetProcessName, Qt::CaseSensitivity::CaseInsensitive)==0) {
				return hwnd;
			}
		}
		QThread::msleep(100);
	}
	return nullptr;
}

static HWND findWindowByProcessName(const QString &targetProcessName) {
	QSet<HWND> windows = getAllVisibleWindows();
	for (HWND hwnd : windows) {
		QString name = getProcessName(hwnd);
		if (name.compare(targetProcessName, Qt::CaseSensitivity::CaseInsensitive) == 0) {
			return hwnd;
		}
	}
	return nullptr;
}

struct EnumData {
	DWORD pid;
	HWND hwnd;
};

LaunchService::LaunchService()
{
}

QString LaunchService::getChromePath()
{
	QSettings settings("HEKY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe",QSettings::Format::NativeFormat);
	QString path = settings.value("Default","").toString();
	if (!path.isEmpty() && QFile::exists(path)) {
		return path;
	}
	QStringList commonPaths = { 
		"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
		"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe",
		QDir::homePath() + "\\APPDATA\\Local\\Google\\Chrome\\Application\\chrome.exe"
	};
	for (QString& path : commonPaths) {
		if (QFile::exists(path)) {
			return path;
		}
	}
	return QString();
}

LaunchService* LaunchService::getInstance()
{
	if (!p_instance)
		p_instance = new LaunchService();
	return p_instance;
}

static RECT rectFromQRect(const QRect& r) {
	RECT rect;
	rect.left = r.x(); rect.top = r.y(); rect.right = r.x() + r.width(); rect.bottom = r.y() + r.height();
	return rect;
}

bool LaunchService::launchExecutable(QString path, MonitorInfo info, AppPosition pos)
{
	return launchExecutable(path, info, pos, QRect());
}

bool LaunchService::launchExecutable(QString path, MonitorInfo info, AppPosition pos, const QRect& zoneRect)
{
	QMutexLocker locker(&m_mutex);
	QSet<HWND> windows = getAllVisibleWindows();
	QString targetProcessName= QFileInfo(path).fileName();

	qint64 pid = 0;
	if (!QProcess::startDetached(path, QStringList(), QString(), &pid)) {
		qWarning() << "Cannot start process" << targetProcessName;
		return false;
	}

	HWND hwnd = waitForMainWindow(windows, targetProcessName);
	if (!hwnd) {
		// Try to find an existing visible window for the process as a fallback (useful when an instance is already open)
		hwnd = findWindowByProcessName(targetProcessName);
		if (!hwnd) {
			qWarning() << "Cannot find main window of" << targetProcessName;
			return false;
		}
	}
	
	RECT monitorRect;
	monitorRect.left = info.availableGeometry.x();
	monitorRect.top = info.availableGeometry.y();
	monitorRect.right = monitorRect.left + info.availableGeometry.width();
	monitorRect.bottom = monitorRect.top + info.availableGeometry.height();
	RECT targetRect = calculateTargetRect(monitorRect, pos);

	if (!zoneRect.isNull()) {
		QRect absoluteZoneRect = zoneRect.translated(
			info.availableGeometry.x(),
			info.availableGeometry.y()
		);

		RECT zr = rectFromQRect(absoluteZoneRect);

		ShowWindow(hwnd, SW_RESTORE);
		SetWindowPos(hwnd, nullptr,
			zr.left,
			zr.top,
			zr.right - zr.left,
			zr.bottom - zr.top,
			SWP_NOZORDER | SWP_NOACTIVATE);

		RedrawWindow(hwnd, &zr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (pos == AppPosition::FULL_SCREEN) {
		SetWindowPos(hwnd, nullptr,
			monitorRect.left,
			monitorRect.top,
			monitorRect.right - monitorRect.left,
			monitorRect.bottom - monitorRect.top,
			SWP_NOZORDER | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_RESTORE);
		ShowWindow(hwnd, SW_MAXIMIZE);

		SetWindowPos(hwnd, HWND_TOP,
			monitorRect.left,
			monitorRect.top,
			monitorRect.right - monitorRect.left,
			monitorRect.bottom - monitorRect.top,
			SWP_SHOWWINDOW);

		SetForegroundWindow(hwnd);
		RedrawWindow(hwnd, &monitorRect, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else {
		SetWindowPos(hwnd, nullptr,
			targetRect.left,
			targetRect.top,
			targetRect.right - targetRect.left,
			targetRect.bottom - targetRect.top,
			SWP_NOZORDER | SWP_NOACTIVATE);

		RedrawWindow(hwnd, &targetRect, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	return true;
}



bool LaunchService::launchURL(QString url, MonitorInfo info, AppPosition pos)
{
	return launchURL(url, info, pos, QRect());
}

bool LaunchService::launchURL(QString url, MonitorInfo info, AppPosition pos, const QRect& zoneRect)
{
	QMutexLocker locker(&m_mutex);
	QString chromePath = getChromePath();
	if (chromePath.isEmpty()) {
		qWarning() << "Cannot find Chrome, Please install Google Chrome";
		return false;
	}

	QSet<HWND> windows = getAllVisibleWindows();

	QStringList arguments = { "--new-window", url };

	qint64 pid = 0;
	if (!QProcess::startDetached(chromePath, arguments, QString(), &pid)) {
		qWarning() << "Cannot start Google Chrome";
		return false;
	}

	HWND hwnd = waitForMainWindow(windows, QString("chrome.exe"));

	if (!hwnd) {
		hwnd = findWindowByProcessName(QString("chrome.exe"));
		if (!hwnd) {
			qWarning() << "Cannot find main window for Google Chrome";
			return false;
		}
	}
	if (IsZoomed(hwnd)) {
		ShowWindow(hwnd, SW_RESTORE);
	}
	RECT monitorRect;
	monitorRect.left = info.availableGeometry.x();
	monitorRect.top = info.availableGeometry.y();
	monitorRect.right = monitorRect.left + info.availableGeometry.width();
	monitorRect.bottom = monitorRect.top + info.availableGeometry.height();
	RECT targetRect = calculateTargetRect(monitorRect, pos);

	if (!zoneRect.isNull()) {
		QRect absoluteZoneRect = zoneRect.translated(
			info.availableGeometry.x(),
			info.availableGeometry.y()
		);

		RECT zr = rectFromQRect(absoluteZoneRect);

		ShowWindow(hwnd, SW_RESTORE);
		SetWindowPos(hwnd, nullptr,
			zr.left,
			zr.top,
			zr.right - zr.left,
			zr.bottom - zr.top,
			SWP_NOZORDER | SWP_NOACTIVATE);

		RedrawWindow(hwnd, &zr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (pos == AppPosition::FULL_SCREEN) {
		SetWindowPos(hwnd, nullptr,
			monitorRect.left,
			monitorRect.top,
			monitorRect.right - monitorRect.left,
			monitorRect.bottom - monitorRect.top,
			SWP_NOZORDER | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_RESTORE);
		ShowWindow(hwnd, SW_MAXIMIZE);

		SetWindowPos(hwnd, HWND_TOP,
			monitorRect.left,
			monitorRect.top,
			monitorRect.right - monitorRect.left,
			monitorRect.bottom - monitorRect.top,
			SWP_SHOWWINDOW);

		SetForegroundWindow(hwnd);
		RedrawWindow(hwnd, &monitorRect, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else {
		SetWindowPos(hwnd, nullptr,
			targetRect.left,
			targetRect.top,
			targetRect.right - targetRect.left,
			targetRect.bottom - targetRect.top,
			SWP_NOZORDER | SWP_NOACTIVATE);

		RedrawWindow(hwnd, &targetRect, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	return true;
}

