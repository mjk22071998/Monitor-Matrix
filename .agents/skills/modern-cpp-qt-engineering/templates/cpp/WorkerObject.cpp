#include "WorkerObject.h"

#include <QThread>
#include <QDebug>

WorkerObject::WorkerObject(QObject* parent)
    : QObject(parent)
{
}

void WorkerObject::doWork(const QString& parameter)
{
    qInfo() << "WorkerObject running in thread:" << QThread::currentThread();

    for (int step = 1; step <= 100; step += 25) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            qInfo() << "WorkerObject received interruption request.";
            emit finished();
            return;
        }

        QThread::msleep(50);
        emit progress(step);
    }

    emit resultReady(QStringLiteral("Processed: %1").arg(parameter));
    emit finished();
}
