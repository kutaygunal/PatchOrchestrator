# Senior Engineer Task — Sprint 38 (S38 / E7 Log export)

You are **senior-engineer-e7**. Implement Sprint 38. Follow `docs/working-rules.md`.

## Sprint 38 (S38) — E7 Log export
- **Scope:** Add a button to export the action log to CSV/JSON for demo handoff.
- **Acceptance criteria:** Export produces valid CSV/JSON file.
- **Dependencies:** E4 (audit log panel).

## Your job
1. Read `docs/phase-e7-tests.md` (test plan) and `docs/sprints-improvements.md` (S38).
2. Read `src/ui/audit_log_panel.hpp/.cpp` (E4). It stores the log as `AuditLogEntry` values
   (action, target, timestamp, result) and renders them in a `QTableWidget`. The timestamp is
   formatted for display (E5) — for export, export the entries' fields.
3. Add a **log export** feature to the audit log panel:
   - Keep a copy of the current log entries in the panel (so export does not depend on parsing
     the table). If not already stored, add a `QList<AuditLogEntry>` member populated by
     `setLog`/`appendEntry`.
   - Add an **Export** `QPushButton` (objectName `exportButton`) in the panel (e.g. a small
     top/toolbar row) and an `exportToFile(const QString &filePath)` method that writes the
     log as **CSV** (header + one row per entry: action, target, timestamp, result) or **JSON**
     (an array of entry objects). Choose CSV for simplicity and correctness (quote/escape
     fields properly). Provide a test accessor `QPushButton *exportButton() const`.
   - Wire the button's click to call `exportToFile` (a `QFileDialog` path is fine for the live
     app, but the method must be directly callable by tests with an explicit path).
4. Add the E7 tests named in `docs/phase-e7-tests.md` under `tests/phase6/`:
   `e7_export_tests.cpp` (T1 — export produces a valid file with all entries and fields),
   `e7_valid_tests.cpp` (T2 — CSV has header + correctly formatted rows, content matches the
   log), and `e7_regression_tests.cpp` (T3 — E4/A1 still pass). Write the files to a temp path
   in the test (e.g. under the build dir) and read them back to assert validity. Register the
   three targets in `tests/phase6/CMakeLists.txt` as `e7_export`, `e7_valid`, and
   `e7_regression`, mirroring the E6 blocks (offscreen `QT_QPA_PLATFORM=offscreen`).
5. Add any new source files to `src/CMakeLists.txt` (both source lists where the other UI
   files appear).
6. Build with a **HARD TIMEOUT** and run the tests **ONE AT A TIME**. Use the VS 2022 dev
   environment for MSVC. **The Qt DLLs are at `C:/Qt/6.8.2/msvc2022_64/bin` — put that on
   PATH or the test binaries fail with `0xc0000135` (DLL not found).**

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing behavior intact so existing tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
