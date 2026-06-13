#include "LayoutManager.h"

#include <qfile.h>
#include <qsavefile.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <qbytearray.h>
#include <quuid.h>

static constexpr quint32 PAYLOAD_MAGIC = 0x50415944; // PAYD

QByteArray LayoutManager::encodePayload(const QByteArray& input)
{
    QByteArray data = qCompress(input, 9);

    const QByteArray key = "ScreenDropLayoutKey_v1";

    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ key[i % key.size()];
    }

    return data;
}

QByteArray LayoutManager::decodePayload(const QByteArray& input)
{
    QByteArray data = input;

    const QByteArray key = "ScreenDropLayoutKey_v1";

    for (int i = 0; i < data.size(); ++i) {
        data[i] = data[i] ^ key[i % key.size()];
    }

    return qUncompress(data);
}

void LayoutManager::writeMonitor(QDataStream& stream, const MonitorInfo& monitor)
{
    stream << monitor.deviceName;
    stream << monitor.model;

    stream << static_cast<qint32>(monitor.geometry.x());
    stream << static_cast<qint32>(monitor.geometry.y());
    stream << static_cast<qint32>(monitor.geometry.width());
    stream << static_cast<qint32>(monitor.geometry.height());

    stream << static_cast<qint32>(monitor.availableGeometry.x());
    stream << static_cast<qint32>(monitor.availableGeometry.y());
    stream << static_cast<qint32>(monitor.availableGeometry.width());
    stream << static_cast<qint32>(monitor.availableGeometry.height());

    stream << static_cast<quint8>(monitor.primary ? 1 : 0);
}

bool LayoutManager::readMonitor(QDataStream& stream, MonitorInfo& monitor, bool hasAvailableGeometry)
{
    QString deviceName;
    QString model;

    qint32 x = 0;
    qint32 y = 0;
    qint32 w = 0;
    qint32 h = 0;

    stream >> deviceName;
    monitor.deviceName = deviceName;

    if (hasAvailableGeometry) {
        stream >> model;
        monitor.model = model;
    }

    stream >> x >> y >> w >> h;
    monitor.geometry = QRect(x, y, w, h);

    if (hasAvailableGeometry) {
        stream >> x >> y >> w >> h;
        monitor.availableGeometry = QRect(x, y, w, h);

        quint8 primary = 0;
        stream >> primary;
        monitor.primary = primary != 0;
    }
    else {
        monitor.availableGeometry = monitor.geometry;
        monitor.primary = false;
    }

    return stream.status() == QDataStream::Ok;
}

bool LayoutManager::saveToFile(
    const QString& filePath,
    const QList<Placement>& placements,
    const QList<MonitorInfo>& monitors)
{
    QByteArray rawPayload;
    QBuffer buffer(&rawPayload);

    if (!buffer.open(QIODevice::WriteOnly)) {
        return false;
    }

    QDataStream payloadStream(&buffer);
    payloadStream.setVersion(QDataStream::Qt_6_2);

    payloadStream << PAYLOAD_MAGIC;

    payloadStream << static_cast<quint32>(monitors.size());

    for (const MonitorInfo& monitor : monitors) {
        writeMonitor(payloadStream, monitor);
    }

    payloadStream << static_cast<quint32>(placements.size());

    for (const Placement& placement : placements) {
        payloadStream << placement.placementId;
        payloadStream << placement.appId;

        writeMonitor(payloadStream, placement.monitor);

        payloadStream << static_cast<quint32>(placement.position);
        payloadStream << static_cast<quint8>(placement.customRect ? 1 : 0);

        if (placement.customRect) {
            payloadStream << static_cast<qint32>(placement.zoneRect.x());
            payloadStream << static_cast<qint32>(placement.zoneRect.y());
            payloadStream << static_cast<qint32>(placement.zoneRect.width());
            payloadStream << static_cast<qint32>(placement.zoneRect.height());
        }
    }

    buffer.close();

    if (payloadStream.status() != QDataStream::Ok) {
        return false;
    }

    QByteArray encodedPayload = encodePayload(rawPayload);

    QSaveFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QDataStream fileStream(&file);
    fileStream.setVersion(QDataStream::Qt_6_2);

    fileStream << s_magicNumber;
    fileStream << s_version;
    fileStream << encodedPayload;

    if (fileStream.status() != QDataStream::Ok) {
        return false;
    }

    return file.commit();
}

bool LayoutManager::loadFromFile(
    const QString& filePath,
    QList<Placement>& savedPlacements,
    QList<MonitorInfo>& savedMonitors)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDataStream fileStream(&file);
    fileStream.setVersion(QDataStream::Qt_6_2);

    quint32 magic = 0;
    quint32 version = 0;

    fileStream >> magic >> version;

    if (magic != s_magicNumber || version < 1 || version > s_version) {
        return false;
    }

    savedPlacements.clear();
    savedMonitors.clear();

    if (version >= 4) {
        QByteArray encodedPayload;
        fileStream >> encodedPayload;

        QByteArray rawPayload = decodePayload(encodedPayload);

        if (rawPayload.isEmpty()) {
            return false;
        }

        QBuffer buffer(&rawPayload);

        if (!buffer.open(QIODevice::ReadOnly)) {
            return false;
        }

        QDataStream stream(&buffer);
        stream.setVersion(QDataStream::Qt_6_2);

        quint32 payloadMagic = 0;
        stream >> payloadMagic;

        if (payloadMagic != PAYLOAD_MAGIC) {
            return false;
        }

        quint32 monitorCount = 0;
        stream >> monitorCount;

        for (quint32 i = 0; i < monitorCount; ++i) {
            MonitorInfo info;

            if (!readMonitor(stream, info, true)) {
                return false;
            }

            savedMonitors.append(info);
        }

        quint32 placementCount = 0;
        stream >> placementCount;

        for (quint32 i = 0; i < placementCount; ++i) {
            QString placementId;
            QString appId;

            stream >> placementId;
            stream >> appId;

            MonitorInfo info;

            if (!readMonitor(stream, info, true)) {
                return false;
            }

            quint32 position = 0;
            stream >> position;

            Placement placement(appId, info, static_cast<AppPosition>(position));
            placement.placementId = placementId;

            quint8 hasCustom = 0;
            stream >> hasCustom;

            if (hasCustom) {
                qint32 x = 0;
                qint32 y = 0;
                qint32 w = 0;
                qint32 h = 0;

                stream >> x >> y >> w >> h;

                placement.zoneRect = QRect(x, y, w, h);
                placement.customRect = true;
            }

            savedPlacements.append(placement);
        }

        return stream.status() == QDataStream::Ok;
    }

    quint32 monitorCount = 0;
    fileStream >> monitorCount;

    for (quint32 i = 0; i < monitorCount; ++i) {
        MonitorInfo info;

        QString deviceName;
        fileStream >> deviceName;

        info.deviceName = deviceName;

        qint32 x = 0;
        qint32 y = 0;
        qint32 w = 0;
        qint32 h = 0;

        fileStream >> x >> y >> w >> h;

        info.geometry = QRect(x, y, w, h);
        info.availableGeometry = info.geometry;

        savedMonitors.append(info);
    }

    quint32 placementCount = 0;
    fileStream >> placementCount;

    for (quint32 i = 0; i < placementCount; ++i) {
        QString placementId;
        QString appId;

        if (version >= 3) {
            fileStream >> placementId;
            fileStream >> appId;
        }
        else {
            fileStream >> appId;
            placementId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }

        MonitorInfo info;

        QString deviceName;
        fileStream >> deviceName;

        info.deviceName = deviceName;

        qint32 x = 0;
        qint32 y = 0;
        qint32 w = 0;
        qint32 h = 0;

        fileStream >> x >> y >> w >> h;
        info.geometry = QRect(x, y, w, h);

        if (version >= 2) {
            fileStream >> x >> y >> w >> h;
            info.availableGeometry = QRect(x, y, w, h);
        }
        else {
            info.availableGeometry = info.geometry;
        }

        quint32 position = 0;
        fileStream >> position;

        Placement placement(appId, info, static_cast<AppPosition>(position));
        placement.placementId = placementId;

        if (version >= 2) {
            quint8 hasCustom = 0;
            fileStream >> hasCustom;

            if (hasCustom) {
                fileStream >> x >> y >> w >> h;

                placement.zoneRect = QRect(x, y, w, h);
                placement.customRect = true;
            }
        }

        savedPlacements.append(placement);
    }

    return fileStream.status() == QDataStream::Ok;
}