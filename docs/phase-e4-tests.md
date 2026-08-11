# Phase E4 — Test Plan (Sprint 35: Audit log panel)

Engineer: `senior-engineer-e4`. Tester: `testing-e4`. This is the test plan for a
`QTableWidget`/`QListView` panel in the demo hub showing the live operator action log.

## Context

Sprint 33 (E2) added the API action log endpoint. Sprint 1 (A1) created the demo hub.
Sprint 35 adds an audit log panel (QTableWidget/QListView) in the demo hub that displays the
live operator action log.

## Acceptance criteria (from sprint)

1. Panel displays log.
2. Updates with new entries.

## Test plan

### T1 — Panel renders log entries

- **Type:** Qt widget test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct the audit log panel with a set of log entries.
- **Assertions:**
  - The panel displays one entry per action.
  - Each entry shows the action, target, timestamp, and result.
  - The number of displayed entries matches the log.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e4_render` (or the equivalent Qt Test binary).

### T2 — Updates with new entries

- **Type:** Qt widget test.
- **Assertions:**
  - Adding a new log entry updates the panel.
  - The new entry appears in the panel.
  - The panel stays in sync with the log.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e4_updates` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt widget test.
- **Assertions:**
  - The demo hub (A1) still works correctly.
  - Existing A1 and E2/E3 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e4_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Add an audit log panel (QTableWidget/QListView) in the demo hub that displays the live
  operator action log.
- Keep the existing demo hub and API intact so A1/E2/E3 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
