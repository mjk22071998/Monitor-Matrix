#include "AppLogger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

QMutex& logMutex()
{
    static QMutex mutex;
    return mutex;
}

QString& activeLogFilePath()
{
    static QString path;
    return path;
}

QString messageTypeName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("DEBUG");
    case QtInfoMsg:
        return QStringLiteral("INFO");
    case QtWarningMsg:
        return QStringLiteral("WARN");
    case QtCriticalMsg:
        return QStringLiteral("CRITICAL");
    case QtFatalMsg:
        return QStringLiteral("FATAL");
    }

    return QStringLiteral("LOG");
}

void writeMessageToDebugger(const QString& line)
{
#ifdef Q_OS_WIN
    OutputDebugStringW(reinterpret_cast<LPCWSTR>(line.utf16()));
    OutputDebugStringW(L"\n");
#else
    Q_UNUSED(line);
#endif
}

void messageHandler(QtMsgType type,
    const QMessageLogContext& context,
    const QString& message)
{
    QMutexLocker locker(&logMutex());

    const QString timestamp =
        QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

    QString location;

    if (context.file && context.line > 0) {
        location = QStringLiteral(" [%1:%2]")
            .arg(QFileInfo(QString::fromUtf8(context.file)).fileName())
            .arg(context.line);
    }

    const QString line = QStringLiteral("%1 [%2]%3 %4")
        .arg(timestamp, messageTypeName(type), location, message);

    QFile file(activeLogFilePath());

    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << line << '\n';
        stream.flush();
    }

    writeMessageToDebugger(line);

    if (type == QtFatalMsg)
        abort();
}

QString fallbackLogDirectory()
{
    QString path = QStandardPaths::writableLocation(
        QStandardPaths::TempLocation
    );

    if (path.isEmpty())
        path = QDir::currentPath();

    return QDir(path).filePath(QStringLiteral("ScreenDropLogs"));
}

} // namespace

void AppLogger::initialize()
{
    QMutexLocker locker(&logMutex());

    QDir dir(logDirectory());

    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    activeLogFilePath() = dir.filePath(QStringLiteral("ScreenDrop.log"));

    QFile file(activeLogFilePath());

    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "\n";
        stream << "==== Screen Drop started "
               << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
               << " ====\n";
        stream << "Application: " << QCoreApplication::applicationFilePath()
               << "\n";
        stream.flush();
    }

    qInstallMessageHandler(messageHandler);
}

QString AppLogger::logDirectory()
{
    QString path = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation
    );

    if (path.isEmpty())
        path = fallbackLogDirectory();

    return QDir(path).filePath(QStringLiteral("logs"));
}

QString AppLogger::logFilePath()
{
    if (!activeLogFilePath().isEmpty())
        return activeLogFilePath();

    return QDir(logDirectory()).filePath(QStringLiteral("ScreenDrop.log"));
}
