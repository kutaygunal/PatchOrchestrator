# Senior Engineer Task — Sprint 37 (S37 / E6 Log auto-refresh)

You are **senior-engineer-e6**. Implement Sprint 37. Follow `docs/working-rules.md`.

## Sprint 37 (S37) — E6 Log auto-refresh
- **Scope:** Make the audit log panel refresh in real time as actions occur (via the B5 stream
  or poll).
- **Acceptance criteria:** Panel auto-refreshes on new actions.
- **Dependencies:** E4 (audit log panel), B5 (real-time status stream).

## Your job
1. Read `docs/phase-e6-tests.md` (test plan) and `docs/sprints-improvements.md` (S37).
2. Read `src/ui/audit_log_panel.hpp/.cpp` (E4) — it has `setLog()` and `appendEntry()`. Read
   `src/ui/dashboard.cpp/.hpp` for how it polls the .NET API (QTimer + QNetworkAccessManager)
   and the B5 SSE stream pattern. The .NET E2 endpoint is `GET /api/schedules/{id}/actions`
   returning the recorded action log.
3. Add **auto-refresh** to the audit log panel so it updates in real time as actions occur:
   - A self-contained `QTimer`-driven poll (and/or a stream hook) that fetches the action log
     from the API and refreshes the panel via `setLog`. Follow the dashboard's pattern.
   - Add control methods so tests can drive refresh deterministically offscreen:
     `void setScheduleId(const QString&)` / `void setApiBaseUrl(const QString&)` (or bind to
     the shared context), `void refreshNow()` (manual immediate fetch), `void setRefreshInterval(int)`
     and `void stopRefresh()` (to stop the timer), plus `int logRowCount()` if not already
     present. The tests construct the panel offscreen and drive it with injected/appended
     entries or a simulated fetch, without needing a live server for the core refresh logic.
   - Keep the E4 `appendEntry`/`setLog` behavior working.
4. Wire the audit log panel's refresh to the shared `DemoAppContext` (A3) in
   `DemoMainWindow` so it uses the active schedule id / API base URL, mirroring how the
   dashboard binds context. Do not break the existing hub.
5. Add the E6 tests named in `docs/phase-e6-tests.md` under `tests/phase6/`:
   `e6_refresh_tests.cpp` (T1 — panel refreshes and shows a new action promptly on an event),
   `e6_multiple_tests.cpp` (T2 — a sequence of events produces a corresponding sequence of
   updates, final state matches latest log), and `e6_regression_tests.cpp` (T3 — E4/B5/A1
   still pass). Register the three targets in `tests/phase6/CMakeLists.txt` as `e6_refresh`,
   `e6_multiple`, and `e6_regression`, mirroring the E5 blocks (offscreen
   `QT_QPA_PLATFORM=offscreen`). These should test the refresh logic directly (e.g. appending
   entries / calling the refresh path) so they pass offscreen without a live server.
6. Add any new source files to `src/CMakeLists.txt` (both source lists where the other UI
   files appear).
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
