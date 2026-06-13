#pragma once

#include <qlist.h>
#include <qstring.h>
#include "../models/Placement.h"

class LayoutManager {
private:
    static constexpr quint32 s_magicNumber = 0x53445250; // SDRP
    static constexpr quint32 s_version = 4;

    static QByteArray encodePayload(const QByteArray& input);
    static QByteArray decodePayload(const QByteArray& input);

    static void writeMonitor(QDataStream& stream, const MonitorInfo& monitor);
    static bool readMonitor(QDataStream& stream, MonitorInfo& monitor, bool hasAvailableGeometry);

public:
    static bool saveToFile(
        const QString& filePath,
        const QList<Placement>& placements,
        const QList<MonitorInfo>& monitors
    );

    static bool loadFromFile(
        const QString& filePath,
        QList<Placement>& placements,
        QList<MonitorInfo>& monitors
    );
};