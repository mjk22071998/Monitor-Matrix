#pragma once

#include <QObject>
#include <QVector>

#include "../models/DisplayConfig.h"

class DisplayConfigService : public QObject {
    Q_OBJECT

public:
    explicit DisplayConfigService(QObject* parent = nullptr);

    QVector<DisplayConfigInfo> getDisplays() const;
    bool applyDisplays(const QVector<DisplayConfigInfo>& displays);

signals:
    void displaysApplied();
    void applyFailed(const QString& message);
};
