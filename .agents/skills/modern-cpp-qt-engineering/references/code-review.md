# Modern C++ & Qt Code Review Checklist

Use this checklist when auditing, reviewing, or submitting C++ and Qt code changes. Every finding must carry a severity label (`[blocking]`, `[should-fix]`, `[question]`, `[nit]`, `[praise]`), file:line evidence, and a verification method.

---

## 1. Memory Safety & Lifetimes

- [ ] **No Raw Owning Pointers**: Are all heap resources owned by `std::unique_ptr`, `std::shared_ptr`, or parent `QObject`?
- [ ] **No Double-Free via Hybrid Ownership**: Is any `QObject` with a non-null parent managed by `std::unique_ptr` or `std::shared_ptr`? (Must be eliminated).
- [ ] **No Dangling References**: Does any function return a reference or pointer to a local variable or temporary?
- [ ] **Lambda Capture Safety**: Does any async lambda capture references (`[&]`) to variables that could fall out of scope before the lambda executes?
- [ ] **QPointer Observation**: Are non-owning pointers to `QObject` instances protected by `QPointer<T>` if the target object can be destroyed during observation?
- [ ] **Resource Leak Check**: Do all exception paths release locks, close file descriptors, and release native handles?

---

## 2. Qt Object Model & Signal/Slot Safety

- [ ] **Modern Connection Syntax**: Are all `connect()` calls using pointer-to-member-function syntax instead of `SIGNAL()`/`SLOT()` macros?
- [ ] **Context Object in Lambda Connections**: Does every `connect(sender, &Signal, [=](){})` provide a context `QObject*` as the third parameter to prevent execution after receiver destruction?
- [ ] **Thread-Safe Deletion**: Is `deleteLater()` used instead of `delete this` within signal handlers or asynchronous callbacks?
- [ ] **QObject Macro**: Does every subclass using signals, slots, or properties declare the `Q_OBJECT` macro?
- [ ] **No Copying QObjects**: Are QObject subclasses kept non-copyable (`Q_DISABLE_COPY`)?

---

## 3. Concurrency, Threading & Synchronization

- [ ] **GUI Thread Exclusivity**: Are all `QWidget` and `QGraphicsItem` operations strictly confined to the main GUI thread?
- [ ] **Thread Affinity**: Are worker objects properly transferred to background threads using `QObject::moveToThread()`?
- [ ] **Safe Inter-Thread Communication**: Are cross-thread interactions mediated via queued signals/slots (`Qt::QueuedConnection`) or atomics?
- [ ] **Lock Order Consistency**: If acquiring multiple mutexes, is `std::scoped_lock` used to prevent deadlocks?
- [ ] **Mutex Recursion**: Is `QRecursiveMutex` used if logging or event processing can re-enter a locked section?
- [ ] **No Busy-Wait Loops**: Are loops waiting on conditions using `QWaitCondition`, `std::condition_variable`, or event loops rather than `while(true) sleep()`?

---

## 4. Modern C++ & Type Safety

- [ ] **Const Correctness**: Are member functions that do not mutate state declared `const`? Are read-only arguments passed by `const&` or `std::string_view`?
- [ ] **Monadic & Explicit Errors**: Are fallible operations returning `std::expected` or `std::optional` instead of raw error codes or sentinel values?
- [ ] **No Implicit Narrowing / C-Style Casts**: Are casts explicit (`static_cast`, `dynamic_cast`, `std::bit_cast`) instead of `(int)x`?
- [ ] **No Undefined Behavior**: Are all primitive variables initialized? Are array indexes bounds-checked?
- [ ] **Move Semantics**: Are sink arguments accepted by value and moved (`std::move`) into position?

---

## 5. Performance & Resource Efficiency

- [ ] **Implicit Sharing Detach Guard**: Are range-based `for` loops over Qt containers guarded with `std::as_const` (`for (const auto& item : std::as_const(list))`)?
- [ ] **String Efficiency**: Are string slices passed as `std::string_view` or `QStringView` rather than allocating new `QString` / `std::string` copies?
- [ ] **Algorithm Complexity**: Are lookups in large collections using `QHash` / `std::unordered_map` ($O(1)$) rather than linear scans ($O(N)$)?
- [ ] **File I/O Buffering**: Is disk I/O buffered rather than repeatedly opening and closing files in hot loops?

---

## 6. Security & Input Validation

- [ ] **Zero Hardcoded Credentials**: Are there any hardcoded passwords, secret keys, or test credentials compiled into the binary?
- [ ] **Input Sanitization**: Are command-line arguments, user-supplied URLs, and file paths properly escaped before launching processes?
- [ ] **Buffer Bounds**: Are Win32 API string buffers allocated with sufficient length and safe against `MAX_PATH` truncation?

---

## 7. Test Adequacy

- [ ] **Unit Tests for Core Logic**: Are all serialization algorithms, mathematical formulas, and parsers accompanied by automated tests?
- [ ] **Signal Assertions**: Are critical state transitions validated using `QSignalSpy`?
- [ ] **Negative / Failure Tests**: Are boundary conditions, empty inputs, invalid JSON, and missing files tested?
