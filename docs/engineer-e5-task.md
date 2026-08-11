# Senior Engineer Task — Sprint 36 (S36 / E5 Timestamp formatting)

You are **senior-engineer-e5**. Implement Sprint 36. Follow `docs/working-rules.md`.

## Sprint 36 (S36) — E5 Timestamp formatting
- **Scope:** Format ISO-8601 timestamps into a human-readable local-time display in the audit
  log panel.
- **Acceptance criteria:** Timestamps formatted to local time; readable.
- **Dependencies:** E4 (audit log panel).

## Your job
1. Read `docs/phase-e5-tests.md` (test plan) and `docs/sprints-improvements.md` (S36).
2. Read `src/ui/audit_log_panel.hpp/.cpp` (E4). The `AuditLogEntry` has a `timestamp` QString
   (ISO-8601 from the .NET API). The panel renders it in the TimestampColumn.
3. Add a **timestamp formatting helper** and apply it in the audit log panel:
   - New self-contained helper (e.g. `src/ui/timestamp_format.hpp/.cpp`, or free functions in
     the panel) that converts an ISO-8601 timestamp string into a human-readable **local-time**
     string (date + time), e.g. `2025-08-11 10:45:12`. Use Qt's `QDateTime::fromString` /
     `fromISO8601Text` (parse as UTC if the string has a Z/offset, then convert to local time
     with `toLocalTime()`), and format with `toString("yyyy-MM-dd HH:mm:ss")`. Handle an
     unparseable/invalid input gracefully (return the original string or a safe fallback).
   - Apply it in the audit log panel so the TimestampColumn displays the formatted local-time
     string instead of the raw ISO-8601 value. Keep the underlying entry's raw timestamp
     intact (format only for display).
4. Add the E5 tests named in `docs/phase-e5-tests.md` under `tests/phase6/`:
   `e5_formatting_tests.cpp` (T1 — each ISO-8601 timestamp formatted to a readable local-time
   string, correct local time), `e5_display_tests.cpp` (T2 — the log panel displays the
   formatted local-time timestamps), and `e5_regression_tests.cpp` (T3 — E4/A1 still pass).
   Register the three targets in `tests/phase6/CMakeLists.txt` as `e5_formatting`, `e5_display`,
   and `e5_regression`, mirroring the E4 blocks (offscreen `QT_QPA_PLATFORM=offscreen`).
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
