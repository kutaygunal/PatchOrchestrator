# Phase E6 — Test Plan (Sprint 37: Log auto-refresh)

Engineer: `senior-engineer-e6`. Tester: `testing-e6`. This is the test plan for the audit log
panel refreshing in real time as actions occur (via the B5 stream or poll).

## Context

Sprint 35 (E4) added the audit log panel. Sprint 15 (B5) added the real-time status stream.
Sprint 37 makes the audit log panel refresh in real time as actions occur (via the B5 stream
or poll).

## Acceptance criteria (from sprint)

1. Panel auto-refreshes on new actions.

## Test plan

### T1 — Refresh on stream event

- **Type:** Qt integration test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct the audit log panel wired to the B5 stream (or poll); emit a new action
  event.
- **Assertions:**
  - The panel refreshes and shows the new action without a manual refresh.
  - The new entry appears in the panel.
  - The refresh happens promptly on the event.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e6_refresh` (or the equivalent Qt Test binary).

### T2 — Multiple events

- **Type:** Qt integration test.
- **Assertions:**
  - A sequence of action events produces a corresponding sequence of updates.
  - The final panel state matches the latest log.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e6_multiple` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt integration test.
- **Assertions:**
  - The audit log panel (E4) still works correctly.
  - Existing A1, B5, and E4 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e6_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Make the audit log panel refresh in real time as actions occur (via the B5 stream or poll).
- Keep the existing log panel and stream intact so A1/B5/E4 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
