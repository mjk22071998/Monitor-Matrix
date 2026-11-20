#pragma once

#include <QObject>
#include <QString>

class WorkerObject : public QObject {
    Q_OBJECT

public:
    explicit WorkerObject(QObject* parent = nullptr);
    ~WorkerObject() override = default;

public slots:
    void doWork(const QString& parameter);

signals:
    void progress(int percentage);
    void resultReady(const QString& result);
    void errorOccurred(const QString& errorMessage);
    void finished();
};
