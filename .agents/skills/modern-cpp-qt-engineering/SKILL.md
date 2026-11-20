---
name: modern-cpp-qt-engineering
description: Use when designing, architecting, implementing, refactoring, building, or reviewing modern C++ and Qt software (C++20/C++23/Qt 6). Enforces modern standards, memory safety, RAII, target-based CMake, and QObject lifecycle patterns.
metadata:
  author: Antigravity Engineering
  version: "1.0.0"
  cpp-standard: "C++23 (default) / C++20"
  qt-version: "6.x (Qt 6.8 LTS / Qt 6.12 LTS)"
  category: engineering
---

# Modern C++ & Qt Engineering Skill

Actionable engineering guidelines for designing, building, and reviewing high-performance, memory-safe C++ and Qt applications.

---

## 1. Core Principles

1. **Safety by Construction**: Prefer compile-time invariants, concepts, strong typing, and RAII over runtime assertions and error codes.
2. **Deterministic Lifetimes**: Every resource, memory block, and handle has exactly one explicit owner. Never pass owning raw pointers.
3. **Target-Based Build Architecture**: Make target-based modern CMake the first-class build model. No global variables, directory-level include pollution, or raw compiler flag dumps.
4. **Qt 6 Idiomatic Patterns**: Honor Qt object tree ownership, prefer pointer-to-member-function connections with context objects, and decouple business logic from the UI.
5. **No Verification, No Claim**: Never claim tests pass or compilation succeeded without running the real commands and inspecting exit codes and diagnostic output.

---

## 2. Hard Rules (Enforced on Every Task)

- **Standard Selection**: Default to the latest officially published standard (**C++23**, ISO/IEC 14882:2024). C++26 is evaluated as a working draft; do not adopt draft features in production without explicit justification.
- **No Silent Downgrade**: Never lower a project's C++ standard (e.g. from C++20/23 to C++17) to bypass compiler diagnostics without explicit user confirmation.
- **Explicit CMake Standards**: Always declare `CXX_STANDARD`, `CXX_STANDARD_REQUIRED ON`, and `CXX_EXTENSIONS OFF` on CMake targets or project level.
- **Smart Pointer Ownership**: Owning allocations must use `std::unique_ptr` (unique ownership), `std::shared_ptr` (shared ownership), or explicit `QObject` parent-child containment.
- **Modern Qt Connections**: Old string-based `SIGNAL()` and `SLOT()` macros are prohibited. Always use pointer-to-member-function syntax. When connecting to a lambda, ALWAYS pass a context `QObject*`.
- **Worker Thread Discipline**: Never subclass `QThread::run()` for business tasks; use worker objects with `QObject::moveToThread()`. Never access GUI widgets from secondary threads.
- **Zero Raw Owning Pointers**: Raw pointers (`T*`) are strictly non-owning observation references.

---

## 3. Mandatory Development Workflow

Every C++ / Qt engineering task must progress through six disciplined phases:

```
Phase A: Project Discovery
        ↓
Phase B: Requirements Clarification
        ↓
Phase C: Technical Design
        ↓
Phase D: Incremental Implementation
        ↓
Phase E: Rigorous Verification
        ↓
Phase F: Completion & Documentation
```

### Phase A — Project Discovery
- Read existing build files (`CMakeLists.txt`, `*.vcxproj`, `vcpkg.json`, `conanfile.txt`).
- Identify the effective C++ standard, Qt minor version, and compiler toolset.
- Inspect architectural structure: locate core business logic, view models, and UI dialogs/views.
- Identify existing technical debt, build warnings, and missing test infrastructure before modifying code.

### Phase B — Requirements Clarification
- Establish the exact functional requirement and runtime contracts.
- Identify ambiguities, boundary conditions, and platform constraints.
- Ask clarifying questions before making irreversible architectural or data schema decisions.
- Do not invent speculative business requirements.

### Phase C — Technical Design
- Define module boundaries, interfaces, and concrete ownership models.
- Select appropriate technologies and libraries adhering to the [Dependency Policy](references/dependency-policy.md).
- Ensure high cohesion, low coupling, and testability.
- Present design decisions and trade-offs for review when tasks have significant architectural scope.

### Phase D — Incremental Implementation
- Write code incrementally following established project conventions.
- Implement RAII resource wrappers, value types, and domain services.
- Adhere to the [Qt Guidelines](references/qt-guidelines.md) and [Architecture Principles](references/architecture.md).
- Avoid modifying unrelated code or imposing stylistic refactors outside task scope.

### Phase E — Rigorous Verification
- Execute the build system (`cmake --build` or MSBuild/Ninja) with diagnostic warnings enabled.
- Run unit and integration tests via `ctest` or test runners.
- Run static analysis (`clang-tidy`) and address warnings.
- Verify thread affinity, exception safety, and memory ownership contracts.
- Re-run verification until all gates pass green.

### Phase F — Completion & Documentation
- Summarize exact changes made and files modified.
- Document build and test outputs with exact commands and exit statuses.
- Record any justified standard deviations or remaining technical debt.

---

## 4. Reference Map

Consult specialized reference guides for in-depth technical specifications:

| Topic | Reference File | Key Coverage |
|---|---|---|
| **Language Standards** | [standards.md](references/standards.md) | C++23/20/17 standards policy, C++26 draft status, feature-test macros, standard selection rules. |
| **Architecture** | [architecture.md](references/architecture.md) | SOLID, modular boundaries, ownership contracts, anti-pattern prohibition. |
| **Qt Engineering** | [qt-guidelines.md](references/qt-guidelines.md) | QObject lifecycle, Signals/Slots, Threading & `moveToThread`, Model/View, Containers. |
| **Modern CMake** | [cmake.md](references/cmake.md) | `qt_standard_project_setup`, target-based configuration, imported targets, compiler flags. |
| **Tooling & Quality** | [tooling.md](references/tooling.md) | Clang-Tidy, Clang-Format, Sanitizers (ASan/UBSan/TSan), Qt Test, GoogleTest. |
| **Dependencies** | [dependency-policy.md](references/dependency-policy.md) | Third-party library selection, evaluation matrix, supply chain security, version verification. |
| **Code Review** | [code-review.md](references/code-review.md) | Defect taxonomy checklist for C++ and Qt (lifetimes, threading, Qt leaks, API misuse). |

---

## 5. Templates

Ready-to-use modern starting points are provided in `templates/`:
- **CMake**: `templates/cmake/CMakeLists.txt` (Modern Qt 6 + C++23 executable and library target setup)
- **Presets**: `templates/cmake/CMakePresets.json` (Structured configure, build, and test presets)
- **Application Entry**: `templates/cpp/main.cpp` (DPI-aware, exception-safe Qt 6 main function)
- **Worker Thread**: `templates/cpp/WorkerObject.h` & `WorkerObject.cpp` (Idiomatic `moveToThread` worker template)
