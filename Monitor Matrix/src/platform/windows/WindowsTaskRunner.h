#pragma once

#include <functional>

namespace WindowsTaskRunner {
void run(std::function<void()> task);
}
