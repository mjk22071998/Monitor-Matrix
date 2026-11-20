# Qt 6 Engineering Guidelines & Best Practices

## 1. QObject Lifecycle & Ownership Contracts

Qt's `QObject` system provides hierarchical parent-child memory management. Understanding how this interacts with modern C++ RAII is critical for memory safety.

### 1.1 The Parent-Child Ownership Tree
- When a `QObject` is constructed with a parent (`new MyObject(parent)`), the parent takes ownership.
- When the parent is destroyed, it automatically deletes all its children.
- **Rule**: Never manage a parented `QObject` with `std::unique_ptr` or `std::shared_ptr`. Doing so causes a **double-free** when the parent destructor executes.

```cpp
// BAD: Double-free hazard!
auto child = std::make_unique<QWidget>(parentWidget); // Danger!

// GOOD: Parent-child hierarchy (Standard Qt Widget pattern)
auto* child = new QWidget(parentWidget); // parentWidget owns child

// GOOD: Root / Standalone object (No parent) managed by RAII
auto rootService = std::make_unique<BackendService>(); // Standalone, unparented
```

### 1.2 QPointer for Safe Observation
- To reference a `QObject` without ownership, use `QPointer<T>`.
- `QPointer<T>` is a guarded pointer that automatically resets to `nullptr` when the referenced `QObject` is destroyed, preventing dangling pointers and use-after-free bugs.

```cpp
QPointer<AppEditDialog> m_activeDialog;

void MainWindow::showDialog() {
    if (!m_activeDialog) {
        m_activeDialog = new AppEditDialog(this);
        m_activeDialog->show();
    } else {
        m_activeDialog->raise();
        m_activeDialog->activateWindow();
    }
}
```

---

## 2. Modern Signals & Slots Syntax

### 2.1 Pointer-to-Member-Function Syntax
Always use the compile-time checked pointer-to-member-function syntax. Old string-based macros (`SIGNAL()`, `SLOT()`) are strictly prohibited because typos fail silently at runtime.

```cpp
// PROHIBITED (Legacy Qt 4/5 string syntax)
connect(button, SIGNAL(clicked()), this, SLOT(onButtonClicked()));

// MANDATORY (Qt 6 compile-time verified)
connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
```

### 2.2 Context Object for Lambda Connections
When connecting a signal to a C++ lambda, **always supply a context `QObject*`**.

```cpp
// DANGEROUS: Lambda runs even after 'this' is destroyed -> Crash / UB!
connect(service, &Service::dataReady, [this](const Data& d) {
    updateUi(d); // Accesses deleted 'this' if MainWindow was closed!
});

// SAFE: Lambda is automatically disconnected if 'this' is destroyed
connect(service, &Service::dataReady, this, [this](const Data& d) {
    updateUi(d);
});
```

---

## 3. Concurrency & Threading in Qt

### 3.1 The Worker-Object Pattern (`moveToThread`)
Never subclass `QThread::run()` for business tasks. Keep `QThread` as an event loop controller, and move a dedicated worker `QObject` to it.

```cpp
// 1. Create worker and thread
auto* thread = new QThread();
auto* worker = new HeavyComputationWorker(); // No parent!

// 2. Move worker to thread
worker->moveToThread(thread);

// 3. Connect lifecycle and signals
QObject::connect(thread, &QThread::started, worker, &HeavyComputationWorker::process);
QObject::connect(worker, &HeavyComputationWorker::finished, thread, &QThread::quit);
QObject::connect(worker, &HeavyComputationWorker::finished, worker, &QObject::deleteLater);
QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);

// 4. Connect result back to GUI context
QObject::connect(worker, &HeavyComputationWorker::resultReady, this, &MainWindow::handleResult);

// 5. Start thread
thread->start();
```

### 3.2 Thread Affinity Rules
1. **GUI Thread Exclusivity**: `QWidget`, `QGraphicsItem`, and all UI elements must only be created, accessed, or modified on the main GUI thread.
2. **Cross-Thread Communication**: Communicate between threads exclusively via signals and slots. Qt will automatically use `Qt::QueuedConnection`, ensuring arguments are safely marshaled across thread boundaries.
3. **Register Custom Types**: If passing custom structs across queued connections, register them with `qRegisterMetaType<MyType>()`.

---

## 4. Qt Containers vs Standard Library

Qt 6 is heavily optimized for interoperability with the C++ standard library.

### 4.1 String Types
- Use `QString` at Qt API boundaries and for user-facing localized text.
- Use `QStringView` or `QUtf8StringView` (C++17/20) for read-only string slicing without memory allocation.
- Use `std::string` or `std::string_view` for internal core business logic independent of Qt.

### 4.2 Implicit Sharing (Copy-On-Write) Pitfall
Qt containers (`QList`, `QMap`, `QHash`) use implicit sharing. Calling non-const methods or iterating with non-const iterators triggers a deep copy detachment.

```cpp
// HAZARD: May trigger an accidental deep copy of 'list'
for (auto& item : list) { ... }

// SAFE: Const iteration does not detach
for (const auto& item : std::as_const(list)) { ... }
```

---

## 5. UI Architecture: Qt Widgets vs Qt Quick (QML)

| Dimension | Qt Widgets | Qt Quick (QML) |
|---|---|---|
| **Best For** | Complex desktop tools, IDEs, CAD, data-dense forms, multi-window utilities. | Fluid mobile/touch UIs, automotive displays, high-FPS animated visualizers. |
| **Styling** | QStyle, QSS (Qt Style Sheets), QPalette. | Custom Shader Effects, State transitions, QML Styling modules. |
| **Rendering** | Software raster / QPainter backed. | Scene Graph (Direct3D 11/12, Vulkan, Metal, OpenGL via RHI). |
| **Logic Binding** | C++ Signals & Slots. | Declarative property bindings + C++ backend models. |

### Model/View Separation
- Subclass `QAbstractItemModel` or `QAbstractListModel` to expose collections to UI views (`QListView`, `QTableView`, QML `ListView`).
- Never store domain state directly inside `QListWidget` or `QTableWidget` items in non-trivial applications.

---

## 6. Testing with Qt Test (`QTest`)

Use `QTest` for unit testing Qt components and `QSignalSpy` for verifying asynchronous signals:

```cpp
#include <QTest>
#include <QSignalSpy>

class TestAppCatalog : public QObject {
    Q_OBJECT
private slots:
    void testAppAddition();
};

void TestAppCatalog::testAppAddition() {
    AppCatalogService catalog;
    QSignalSpy spy(&catalog, &AppCatalogService::applicationsChanged);

    QVERIFY(catalog.addApplication("test.exe", "Test"));
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestAppCatalog)
#include "test_app_catalog.moc"
```

---

## 7. Deprecated APIs to Avoid in Qt 6

- Avoid `QRegExp` → use `QRegularExpression`.
- Avoid `endl` → use `Qt::endl`.
- Avoid `QTextCodec` → use `QStringConverter`.
- Avoid `QProcess::start(QString)` with combined command line → use `QProcess::start(QString, QStringList)` or `startCommand()`.
- Avoid `QVector` as a distinct type → in Qt 6, `QVector` is an alias for `QList`.
