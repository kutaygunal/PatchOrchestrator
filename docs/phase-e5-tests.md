# Phase E5 — Test Plan (Sprint 36: Timestamp formatting)

Engineer: `senior-engineer-e5`. Tester: `testing-e5`. This is the test plan for formatting
ISO-8601 timestamps into a human-readable local-time display in the log panel.

## Context

Sprint 35 (E4) added the audit log panel. Sprint 36 formats ISO-8601 timestamps into a
human-readable local-time display in the log panel.

## Acceptance criteria (from sprint)

1. Timestamps formatted to local time.
2. Readable.

## Test plan

### T1 — Timestamp formatting

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Provide ISO-8601 timestamps.
- **Assertions:**
  - Each ISO-8601 timestamp is formatted into a human-readable local-time string.
  - The formatted string is readable (e.g. date + time).
  - The local time is correct for the given timestamp.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e5_formatting` (or the equivalent Qt Test binary).

### T2 — Readable display

- **Type:** Qt widget test.
- **Assertions:**
  - The log panel displays timestamps in the human-readable local-time format.
  - The formatted timestamps are clearly readable.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e5_display` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt unit test.
- **Assertions:**
  - The audit log panel (E4) still works correctly.
  - Existing A1 and E4 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e5_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Format ISO-8601 timestamps into a human-readable local-time display in the log panel.
- Keep the existing log panel intact so A1/E4 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
