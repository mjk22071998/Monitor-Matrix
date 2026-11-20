# Dependency & Technology Selection Policy

## 1. Guiding Principles

Every third-party dependency introduced into a C++ or Qt project carries a lifetime maintenance, licensing, security, and build-time cost.

### Core Rules
1. **Prefer Standard & Qt Built-ins**: Exhaust capabilities in the C++ standard library (`std::format`, `std::expected`, `std::ranges`) and Qt modules (`QtCore`, `QtGui`, `QtNetwork`, `QtSql`) before evaluating external packages.
2. **Zero Unverified Dependencies**: Never introduce a library without verifying its official release version, repository maintenance health, and compatibility with the project's toolchain.
3. **No Unapproved Pre-Releases**: Never introduce alpha, beta, RC, nightly, or experimental dependencies into production code without explicit user consent.
4. **Reproducible Package Management**: Manage dependencies via `vcpkg` (manifest mode `vcpkg.json`), `Conan 2.x`, or CMake `FetchContent` with cryptographic checksums and pinned release tags.

---

## 2. Evaluation Matrix for Third-Party Libraries

Before adding any dependency, evaluate against these five criteria:

| Dimension | Evaluation Criteria | Pass Threshold |
|---|---|---|
| **1. Maintenance** | Commit frequency, issue triage, recent stable release within the last 12 months. | Active maintenance, verified security updates. |
| **2. Licensing** | License compatibility with project commercial / open-source distribution. | Permissive (MIT, BSD, Apache-2.0) or compliant LGPL-3.0. |
| **3. C++ Standard** | Compatibility with C++20/C++23 standard library implementations. | Compiles without compiler diagnostics or warnings under `/W4` / `-Wall`. |
| **4. Transitive Bloat** | Number of downstream dependencies pulled into the build. | Minimal dependency tree; self-contained. |
| **5. Portability** | Supports all target operating systems (Windows MSVC, Linux GCC/Clang, macOS). | Cross-platform compatibility verified. |

---

## 3. Package Management Best Practices

### 3.1 vcpkg Manifest Mode (`vcpkg.json`)
Prefer declarative manifest mode over classic imperative installation:

```json
{
  "name": "screen-drop",
  "version-string": "1.0.0",
  "dependencies": [
    "fmt",
    "nlohmann-json"
  ],
  "builtin-baseline": "3426027429188d2642e360980226490dd7074463"
}
```

Integrate seamlessly into CMake:
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake
```

### 3.2 CMake `FetchContent` Guardrails
When using `FetchContent` for lightweight, header-only or standalone libraries:
- **Always pin an immutable git tag or commit hash** (never track `main` or `master`).
- **Always specify `GIT_SHALLOW TRUE`** to minimize download bandwidth.
- **Set target scope to `EXCLUDE_FROM_ALL`** so dependency targets are not built unnecessarily.

```cmake
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)
```

---

## 4. Documentation of Decisions

Whenever an external dependency is introduced, document:
- **Library Name & Version**: The exact version evaluated.
- **Need / Justification**: What concrete requirement necessitates this dependency over standard C++ or Qt.
- **Alternatives Evaluated**: What options were considered and why this library was selected.
- **License**: Confirmed license model.
