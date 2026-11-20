# Tooling, Static Analysis & Testing Reference

## 1. Compiler Toolchains

Modern C++23 development requires modern compilers and standard library implementations:

| Toolchain | Recommended Baseline | C++23 Support | Windows / Linux / macOS |
|---|---|---|---|
| **GCC** | 14.0+ | Broad C++23 core & stdlib | Primary Linux / MinGW |
| **Clang** | 18.0+ | Broad C++23 core | Primary macOS / Linux / clang-cl |
| **MSVC** | 19.40+ (VS 2022 v17.10+ / VS 2026 v145) | Strong `/std:c++23` and `/std:c++latest` | Primary Windows |

### Recommended Generator: Ninja
Always prefer **Ninja** over Make or MSBuild for CMake builds:
- Fast multi-core dependency evaluation.
- Clean diagnostic output.
- Interoperates with `compile_commands.json` for language servers and Clang-Tidy.

---

## 2. Static Analysis: Clang-Tidy

Run Clang-Tidy as a standard quality gate during development.

### 2.1 Recommended `.clang-tidy` Configuration
```yaml
---
Checks: >
  -*,
  bugprone-*,
  cppcoreguidelines-*,
  misc-*,
  modernize-*,
  performance-*,
  readability-*,-readability-magic-numbers

WarningsAsErrors: 'bugprone-*,cppcoreguidelines-*,performance-*'
HeaderFilterRegex: 'src/.*'
FormatStyle: file
```

### 2.2 Integration with CMake
```cmake
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()
```

---

## 3. Code Formatting: Clang-Format

Format all code with `clang-format` using a project `.clang-format` file:

```yaml
---
BasedOnStyle: Mozilla
Standard: Latest
ColumnLimit: 100
IndentWidth: 4
TabWidth: 4
UseTab: Never
BreakBeforeBraces: Attach
AllowShortFunctionsOnASingleLine: Inline
FixNamespaceComments: true
IncludeBlocks: Regroup
SortIncludes: CaseSensitive
```

Run via CLI:
```bash
clang-format -i --style=file src/**/*.cpp src/**/*.h
```

---

## 4. Runtime Sanitizers

Use runtime sanitizers during Debug and Test configurations to trap memory and concurrency defects:

### 4.1 AddressSanitizer (ASan) & LeakSanitizer
- **Detects**: Out-of-bounds access, heap/stack use-after-free, double-free, memory leaks.
- **GCC / Clang**: `-fsanitize=address,leak -fno-omit-frame-pointer`
- **MSVC**: `/fsanitize=address`

### 4.2 UndefinedBehaviorSanitizer (UBSan)
- **Detects**: Integer overflow, null-pointer dereferences, misaligned pointers, bitshift out-of-range.
- **GCC / Clang**: `-fsanitize=undefined`

### 4.3 ThreadSanitizer (TSan)
- **Detects**: Data races between threads, lock order inversions (deadlocks).
- **GCC / Clang**: `-fsanitize=thread`
- *Note*: Cannot be combined with ASan in a single build. Maintain separate presets.

---

## 5. Testing Frameworks

### 5.1 Qt Test Framework (`QTest`)
- Use `QTest` for any test requiring a `QCoreApplication`, `QEventLoop`, or UI widgets.
- Use `QSignalSpy` to assert that signals fire with expected arguments:
  ```cpp
  QSignalSpy spy(sender, &Sender::statusChanged);
  sender->trigger();
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.takeFirst().at(0).toString(), QString("OK"));
  ```

### 5.2 CTest Orchestration
Register all unit tests in CMake:

```cmake
enable_testing()

add_executable(test_layout_manager tests/test_layout_manager.cpp)
target_link_libraries(test_layout_manager PRIVATE Qt6::Test ScreenDropCore)
add_test(NAME LayoutManagerTest COMMAND test_layout_manager)
```

Run tests from CLI:
```bash
ctest --output-on-failure --test-dir build
```
