# Code Modification Guidelines

## 1. Git Branch Protection

- Never modify code directly on the `main` branch.
- Before editing, check the current branch:

```bash
git branch --show-current
````

* If the result is `main`, create or switch to a development branch first:

```bash
git checkout -b feature/xxx
```

* Do not bypass branch protection by:

  * Committing directly to `main`.
  * Running code generation, formatting, or large refactors on `main`.
  * Rewriting or damaging `main` history.

---

## 2. Code Comments

* All new comments must be written in English.
* Comments must be:

  * Short and clear.
  * Explain the purpose, not repeat the code.
  * Avoid unnecessary details.

Recommended:

```cpp
// AI-generated: Prevent stale handle access.
```

Avoid:

```cpp
// AI-generated: This checks if the handle is invalid because...
```

* Important AI-added or modified logic should include:

```cpp
// AI-generated
```

* Do not add comments for trivial changes such as:

  * One or two line logic fixes.
  * Spelling corrections.
  * Simple variable renames.

### 2.1 Do not write excessive or granular comments

AI must avoid sprinkling small explanatory comments throughout code.
Specifically:

* **No line-by-line narration.** Do not add a comment above (or beside)
  every field, signal, slot, branch, or block just to restate what it is.
  If the name is self-explanatory, no comment is needed.
* **No multi-part explanations on a single marker.** A line like
  `// AI-generated: abort the in-flight download; finished handler clears state.`
  is too much. Either drop it, or shorten to a single short clause
  (e.g. `// AI-generated: abort in-flight download.`).
* **Prefer zero or one comment per file/section**, not many. A short
  header comment explaining the file's purpose is acceptable; dozens of
  inline labels are not.
* **Do not comment obvious QML/C++ bindings, layout splits, or default
  values** (e.g. `// AI-generated: index 0 -> zh_CN, index 1 -> en.`).
  The code already says that.
* **When in doubt, omit the comment.** A missing comment is fine; a
  redundant one is noise that future maintainers must clean up.

Rule of thumb: if removing the comment would not lose useful
information, remove it.

---

## 3. Project Style

Follow the existing project style strictly:

* Naming conventions.
* File organization.
* Formatting rules.
* Include order.
* Existing architecture.

Do not:

* Introduce unrelated coding styles.
* Add unnecessary design patterns.
* Create duplicate systems.
* Add abstractions without need.
* Change module responsibilities.

---

## 4. Modification Scope

Only modify the minimum code required.

Do not:

* Format unrelated files.
* Remove existing comments.
* Rename large numbers of symbols.
* Change public APIs.
* Perform large refactors without confirmation.

If large changes are required, explain first:

1. Current problem.
2. Required changes.
3. Affected modules.
4. Expected impact.

---

## 5. Build and Test

After modifications:

* Verify compilation.
* Check for new warnings.
* Ensure existing features still work.

Do not:

* Commit without verification.
* Ignore compiler errors.
* Hide problems with temporary hacks.

---

## 6. Compatibility

Consider:

* Existing callers.
* API/ABI compatibility.
* Serialization formats.
* Network protocols.
* Save data.
* Threading impact.

Do not:

* Change network structures without compatibility handling.
* Break old data formats.
* Change public data layouts carelessly.

---

## 7. Multi-threading Safety

For multi-threaded code, check:

* Data races.
* Object lifetime.
* Lock contention.
* Atomic correctness.
* Cross-thread resource access.

Do not:

* Assume single-threaded execution.
* Modify shared data without protection.
* Add hidden locks that hurt performance.

---

## 8. Memory Safety

Consider:

* Ownership.
* Lifetime.
* RAII.
* Memory leaks.
* Dangling references.
* Iterator invalidation.

Do not:

* Return references to local variables.
* Store pointers to temporary objects.
* Use uninitialized data.
* Introduce undefined behavior.

---

## 9. Third-party Libraries

Before adding or changing dependencies:

Check:

* Existing project dependencies.
* Build system impact.
* Maintenance cost.

Do not:

* Add large libraries for simple features.
* Duplicate existing library functionality.
* Modify third-party source code.

---

## 10. AI Behavior Rules

AI-generated code must:

* Reuse existing code first.
* Preserve current architecture.
* Prefer simple solutions.
* Minimize risk and code changes.

Do not:

* Guess requirements.
* Add unrequested features.
* Redesign architecture.
* Add unnecessary abstraction layers.

Choose the implementation with:

> The smallest change, lowest risk, and best compatibility with existing code.

---

## 11. Pre-commit Checklist

Before committing:

* [ ] Not on `main`.
* [ ] Changes match the requested task.
* [ ] New comments are in English.
* [ ] AI changes are marked when needed.
* [ ] No unrelated formatting changes.
* [ ] Build passes.
* [ ] No obvious performance/security/thread issues.
* [ ] No API, protocol, or data format breakage.

