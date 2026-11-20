#pragma once

#include <QObject>
#include <QVector>
#include <QVariantList>
#include <QVariantMap>

#include "../models/DisplayConfig.h"
#include "../services/DisplayConfigService.h"

class DisplayConfigBridge : public QObject {
    Q_OBJECT

    Q_PROPERTY(int displayCount READ displayCount NOTIFY displaysChanged)
    Q_PROPERTY(bool hasChanges READ hasChanges NOTIFY hasChangesChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    explicit DisplayConfigBridge(QObject* parent = nullptr);
    ~DisplayConfigBridge() override = default;

    int displayCount() const { return m_displays.size(); }
    bool hasChanges() const { return m_hasChanges; }

    int selectedIndex() const { return m_selectedIndex; }
    void setSelectedIndex(int index);

    Q_INVOKABLE void refreshDisplays();
    Q_INVOKABLE QVariantList displays() const;
    Q_INVOKABLE QVariantMap getDisplay(int index) const;
    Q_INVOKABLE QVariantList getResolutions(int displayIndex) const;

    Q_INVOKABLE bool setDisplayPosition(int displayIndex, int x, int y);
    Q_INVOKABLE bool setDisplayResolution(int displayIndex, int width, int height);
    Q_INVOKABLE bool setPrimaryDisplay(int displayIndex);

    Q_INVOKABLE bool applyChanges();
    Q_INVOKABLE void revertChanges();

signals:
    void displaysChanged();
    void hasChangesChanged(bool hasChanges);
    void selectedIndexChanged(int index);
    void applySuccess();
    void applyFailed(const QString& errorMessage);

private:
    void updateChangeState();
    bool computeHasChanges() const;

    DisplayConfigService m_service;
    QVector<DisplayConfigInfo> m_displays;
    QVector<DisplayConfigInfo> m_originalDisplays;

    int m_selectedIndex = -1;
    bool m_hasChanges = false;
    QString m_lastErrorMessage;
};
