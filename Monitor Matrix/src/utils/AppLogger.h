#pragma once

#include <QString>

class AppLogger {
public:
    static void initialize();
    static QString logDirectory();
    static QString logFilePath();
};
