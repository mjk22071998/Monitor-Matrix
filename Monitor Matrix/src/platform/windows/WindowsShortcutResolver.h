#pragma once

#include <qstring.h>

namespace WindowsShortcutResolver {

bool resolveExecutableTarget(
    const QString& shortcutPath,
    QString* executablePath,
    QString* errorMessage = nullptr
);

} // namespace WindowsShortcutResolver
