# Architecture & Code Quality Principles

## 1. Core Architectural Pillars

A maintainable C++ / Qt codebase is structured around strict layer boundaries, high cohesion, low coupling, and explicit ownership contracts.

```
┌────────────────────────────────────────────────────────┐
│             Presentation Layer (UI / Views)            │
│       Qt Widgets (QMainWindow, QDialog) / QML Items    │
└───────────────────────────┬────────────────────────────┘
                            │ Signals / Slots / Commands
┌───────────────────────────▼────────────────────────────┐
│          Service / Application Layer (Use Cases)       │
│      ProcessLaunchService, TopologyManager, Importers  │
└───────────────────────────┬────────────────────────────┘
                            │ Pure C++ / Abstract Interfaces
┌───────────────────────────▼────────────────────────────┐
│              Domain & Model Layer (State)              │
│       Placement, MonitorInfo, Value Objects, Logic     │
└───────────────────────────┬────────────────────────────┘
                            │ Interfaces
┌───────────────────────────▼────────────────────────────┐
│          Infrastructure & Platform Layer (OS)          │
│        Win32 APIs, QSettings, File Storage, Network    │
└────────────────────────────────────────────────────────┘
```

---

## 2. SOLID Principles in C++ & Qt

### 2.1 Single Responsibility Principle (SRP)
- A class must have only one reason to change.
- **Rule**: Never mix UI rendering, network communication, and business computation in the same class.
- *Bad*: `MainWindow` reads configuration files, makes Win32 display calls, launches processes, and renders graphics items.
- *Good*: `MainWindow` manages UI presentation; `DisplayTopologyService` queries display hardware; `ConfigManager` reads JSON; `LayoutLaunchService` launches processes.

### 2.2 Open/Closed Principle (OCP)
- Classes should be open for extension, but closed for modification.
- **Rule**: Use polymorphic strategy interfaces or `std::variant` dispatch to add new behaviors without editing existing callers.
- *Example*: `LaunchStrategy` base class allows adding `FlatpakStrategy` or `WaylandStrategy` without altering `LayoutLaunchService`.

### 2.3 Liskov Substitution Principle (LSP)
- Derived classes must be substitutable for their base classes without violating invariants or contracts.
- **Rule**: Never override a virtual function with an empty stub that throws or silently ignores operations. Never violate the base class postconditions.

### 2.4 Interface Segregation Principle (ISP)
- Clients should not be forced to depend on methods they do not use.
- **Rule**: Split monolithic interfaces into fine-grained abstract protocols:
  ```cpp
  class IConfigReader {
  public:
      virtual ~IConfigReader() = default;
      [[nodiscard]] virtual std::optional<AppConfig> read(std::string_view path) = 0;
  };

  class IConfigWriter {
  public:
      virtual ~IConfigWriter() = default;
      [[nodiscard]] virtual bool write(std::string_view path, const AppConfig& config) = 0;
  };
  ```

### 2.5 Dependency Inversion Principle (DIP)
- High-level modules must not depend on low-level modules; both must depend on abstractions.
- **Rule**: Pass dependencies via constructors (Dependency Injection) rather than instantiating hardcoded concrete types or calling global singletons.

---

## 3. Practical Pragmatism: DRY, KISS, and YAGNI

1. **DRY (Don't Repeat Yourself)**:
   - Factor common algorithms (e.g. geometric snapping, string escaping, rect clamping) into pure, testable utility functions.
   - Do not duplicate data structures between serialization formats and domain models unless there is a versioning requirement.
2. **KISS (Keep It Simple, Stupid)**:
   - Prefer simple, readable value types over elaborate template metaprogramming or deeply nested class hierarchies.
   - Use standard C++ library features before writing custom container wrappers.
3. **YAGNI (You Aren't Gonna Need It)**:
   - Do not introduce plugin architectures, reflection registries, or multi-tenant abstractions unless explicitly required.
   - Implement the smallest, cleanest design that satisfies the immediate specification.

---

## 4. Prohibited Anti-Patterns

| Anti-Pattern | Description | Remediation |
|---|---|---|
| **God Class** | A single class with > 1,000 lines taking responsibility for multiple subsystems. | Decompose into focused services, models, and presentation controllers. |
| **Unnecessary Singleton** | `getInstance()` everywhere holding mutable global state. | Use dependency injection: pass scoped service references or smart pointers into constructors. |
| **Deep Inheritance** | Hierarchies 3+ levels deep (`BaseWidget` → `CustomWidget` → `EditableWidget` → `AppWidget`). | Prefer composition: embed helper objects or delegate behaviors. |
| **Circular Includes** | Header A includes Header B which includes Header A. | Use forward declarations (`class MyClass;`) in header files; `#include` only in `.cpp`. |
| **Raw Owning Pointer** | `MyClass* ptr = new MyClass();` stored as a member. | Use `std::unique_ptr<MyClass>` or assign an explicit parent `QObject`. |
| **Global Mutable State** | `static int g_counter;` or global singletons without thread synchronization. | Encapsulate state inside an application context object owned by `main()`. |
| **Macro Abuse** | `#define MAX_BUFFER 1024` or macros for calculations. | Use `constexpr auto MaxBufferSize = 1024;` and inline template functions. |
| **Hardcoded Secrets** | Credentials or fixed paths embedded in source code. | Move to secure external stores, environment variables, or user configuration files. |
| **Giant Source Files** | `.cpp` files with 1,500+ lines covering unrelated utility methods. | Break into cohesive translation units grouped by subsystem namespace. |

---

## 5. Header Hygiene & Translation Unit Decoupling

1. **Include What You Use (IWYU)**: Every header must `#include` the types it needs to be self-sufficient and compile in isolation.
2. **Forward Declarations**: Forward declare pointers and references in headers (`class QWidget; struct MonitorInfo;`) to reduce compilation dependencies and build times.
3. **Pragma Once**: Always begin headers with `#pragma once`.
4. **Anonymous Namespaces**: Place file-local helper functions and constants in anonymous namespaces inside `.cpp` files to give them internal linkage and avoid symbol collisions.
