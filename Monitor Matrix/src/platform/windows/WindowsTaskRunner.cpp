#include "WindowsTaskRunner.h"

#include <QDebug>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {

#ifdef Q_OS_WIN
void CALLBACK runTaskCallback(PTP_CALLBACK_INSTANCE, void* context)
{
    auto* task = static_cast<std::function<void()>*>(context);

    if (task) {
        (*task)();
        delete task;
    }
}
#endif

}

void WindowsTaskRunner::run(std::function<void()> task)
{
#ifdef Q_OS_WIN
    auto* heapTask = new std::function<void()>(std::move(task));

    if (!TrySubmitThreadpoolCallback(runTaskCallback, heapTask, nullptr)) {
        qWarning() << "TrySubmitThreadpoolCallback failed. error=" << GetLastError();
        (*heapTask)();
        delete heapTask;
    }
#else
    task();
#endif
}
