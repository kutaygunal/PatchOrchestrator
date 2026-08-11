# Senior Engineer Task — Sprint 35 (S35 / E4 Audit log panel)

You are **senior-engineer-e4**. Implement Sprint 35. Follow `docs/working-rules.md`.

## Sprint 35 (S35) — E4 Audit log panel
- **Scope:** A `QTableWidget`/`QListView` panel in the demo hub showing the live operator
  action log.
- **Acceptance criteria:** Panel displays log; updates with new entries.
- **Dependencies:** E2/E3 (action log API), A1 (demo hub).

## Your job
1. Read `docs/phase-e4-tests.md` (test plan) and `docs/sprints-improvements.md` (S35).
2. Read the demo hub `src/ui/demo_main_window.cpp/.hpp` (A1/A2: embeds panels as tabs, binds
   a shared `DemoAppContext`), and a self-contained UI widget pattern like
   `src/ui/fleet_summary_panel.cpp/.hpp` (C4) or the D1/D2/D3 controls. The operator action
   log fields (from the .NET E1/E2 `ActionLogEntry`) are: **action**, **target**,
   **timestamp**, **result**.
3. Add a new **AuditLogPanel** widget (`src/ui/audit_log_panel.hpp/.cpp`) following the
   established self-contained pattern:
   - A `QTableWidget` (or `QListView` + model) with columns for action, target, timestamp,
     and result.
   - A simple entry type (e.g. a small struct `AuditLogEntry { action, target, timestamp,
     result }`) and methods to set/replace the whole log and to append new entries, so the
     panel renders one row per entry and stays in sync.
   - Constructor `explicit AuditLogPanel(QWidget *parent = nullptr)`.
   - Test accessors: the table (e.g. `QTableWidget *table() const`), row count, and the
     displayed cell text for a given row/column.
4. Wire `AuditLogPanel` into `DemoMainWindow` (`src/ui/demo_main_window.cpp/.hpp`): add it as
   a new tab "Audit Log" (and a member + accessor `AuditLogPanel *auditLog() const`), mirroring
   how the other tabs are added. It should display the action log; for the widget test the
   panel can be driven directly via its append/set methods (no live server needed).
5. Add the new source files to `src/CMakeLists.txt` (both source lists where the other UI
   panels appear).
6. Write the E4 tests named in `docs/phase-e4-tests.md` under `tests/phase6/`:
   `e4_render_tests.cpp` (T1 — panel renders log entries), `e4_updates_tests.cpp` (T2 —
   updates with new entries), and `e4_regression_tests.cpp` (T3 — A1/E2/E3 still pass).
   Register the three targets in `tests/phase6/CMakeLists.txt` as `e4_render`, `e4_updates`,
   and `e4_regression`, mirroring the D5 blocks (offscreen `QT_QPA_PLATFORM=offscreen`).
7. Build with a **HARD TIMEOUT** and run the tests **ONE AT A TIME**. Use the VS 2022 dev
   environment for MSVC. **The Qt DLLs are at `C:/Qt/6.8.2/msvc2022_64/bin` — put that on
   PATH or the test binaries fail with `0xc0000135` (DLL not found).**

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing behavior intact so existing tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
