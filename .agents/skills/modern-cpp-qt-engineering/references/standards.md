# C++ Language Standards & Compatibility Policy

## 1. Standards Status & Baseline Policy

Engineering decisions must be anchored on official ISO standard publication status:

| Standard | ISO Designation | Status | Engineering Policy |
|---|---|---|---|
| **C++23** | ISO/IEC 14882:2024 | **Published Standard** (Oct 2024) | **Default standard for greenfield projects.** Adopt core features whenever supported by toolchain. |
| **C++20** | ISO/IEC 14882:2020 | **Published Standard** (Dec 2020) | Stable baseline for enterprise toolchains or LTS systems. |
| **C++17** | ISO/IEC 14882:2017 | **Published Standard** (Dec 2017) | Maintenance mode only. Do not adopt for new projects unless constrained by legacy toolchains. |
| **C++26** | Working Draft / DIS | **In-Development Draft** | **Experimental.** Do not adopt draft features in production without explicit approval. |

---

## 2. Standard Selection & Downgrade Rules

1. **Greenfield Default**: All new projects default to **C++23**.
2. **Compiler Verification**: Before setting the standard, verify compiler and standard library support:
   - GCC: 14.0+ provides broad C++23 library & core support.
   - Clang: 18.0+ provides solid C++23 support.
   - MSVC: 19.40+ (Visual Studio 2022 v17.10+ / VS 2026 v145) under `/std:c++latest` or `/std:c++23`.
3. **Fallback Policy**: If the target compiler cannot support C++23 (e.g. GCC 11 or MSVC 2019), fall back to **C++20**. Document this decision in the project README or architecture notes.
4. **Strict Downgrade Prohibition**:
   - **NEVER silently downgrade a project's C++ standard.**
   - If a compile error occurs due to an unsupported feature, fix the code to match the declared standard or ask the user for authorization before downgrading.
5. **No Extensions**: Always disable non-standard compiler extensions (`-fno-permissive`, `/permissive-`, `CMAKE_CXX_EXTENSIONS OFF`).

---

## 3. CMake Standards Configuration

Always declare language standards explicitly using target-based properties:

```cmake
# Target-based specification (Preferred)
target_compile_features(my_target
    PUBLIC
        cxx_std_23
)

set_target_properties(my_target PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
```

At the top-level `CMakeLists.txt`, set defaults for submodules:

```cmake
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

---

## 4. Modern C++ Feature Adoption Matrix

### 4.1 Resource Management & Lifetimes
- **RAII Everywhere**: No manual resource release. Every OS handle, socket, memory block, and lock must be managed by an RAII guard (`std::unique_ptr`, `std::lock_guard`, custom handle classes).
- **Smart Pointers**:
  - `std::unique_ptr<T>`: Default owning pointer. Zero runtime overhead.
  - `std::shared_ptr<T>`: Use only when lifetime ownership is genuinely shared and cannot be structured hierarchically.
  - `std::weak_ptr<T>`: Break circular references and observe caches.
- **Raw Pointers**: Strictly non-owning observer pointers. Can be `nullptr`. Must never be deleted.
- **References**: Use `const T&` for non-null observation of large types; use `T&` for mutable in-out parameters when unavoidable.

### 4.2 Modern Error Handling
- **`std::expected<T, E>` (C++23)**:
  Use for anticipated, recoverable operational failures (network errors, file parsing, validation).
  ```cpp
  [[nodiscard]] std::expected<ConfigData, ConfigError> loadConfig(std::string_view path);
  ```
- **`std::optional<T>` (C++17/23)**:
  Use for values that may or may not exist. Use C++23 monadic operations (`and_then()`, `transform()`, `or_else()`):
  ```cpp
  std::optional<User> findUser(UserId id);
  auto name = findUser(id)
      .transform(&User::name)
      .value_or("Anonymous");
  ```
- **Exceptions**: Use exclusively for exceptional, unrecoverable invariant violations (out of memory, critical subsystem missing, programmer bug). Maintain the *strong exception safety guarantee* where practical.

### 4.3 Move Semantics & Forwarding
- Accept sinks by value and `std::move`:
  ```cpp
  explicit UserProfile(std::string displayName)
      : m_name(std::move(displayName)) {}
  ```
- Use `std::forward<T>` only in forwarding references (`template<typename T> void process(T&& arg)`).
- Never `std::move` from `const` objects (silently degrades into a copy).

### 4.4 Concepts & Compile-Time Programming
- Replace complex SFINAE (`std::enable_if_t`) with C++20 **Concepts**:
  ```cpp
  template<typename T>
  concept Serializable = requires(T a, QDataStream& stream) {
      { a.serialize(stream) } -> std::same_as<void>;
  };

  template<Serializable T>
  void writeToStorage(const T& item);
  ```
- Prefer `constexpr` functions and `consteval` immediate functions for compile-time lookup tables and validation.

### 4.5 Ranges & Views
- Prefer `std::ranges` algorithms over iterator pairs:
  ```cpp
  std::ranges::sort(users, {}, &User::name);
  ```
- Use `std::views` for non-allocating, lazy data transformations:
  ```cpp
  auto activeUserNames = users
      | std::views::filter(&User::isActive)
      | std::views::transform(&User::name);
  ```

### 4.6 Concurrency & Synchronization
- Use `std::jthread` (C++20) for threads: automatically joins on destruction and supports cooperative cancellation via `std::stop_token`.
- Use `std::atomic<T>` for lock-free counters and flags.
- Use `std::scoped_lock` to acquire multiple mutexes simultaneously without deadlock risks.
