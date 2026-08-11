# Phase E7 — Test Plan (Sprint 38: Log export)

Engineer: `senior-engineer-e7`. Tester: `testing-e7`. This is the test plan for a button that
exports the action log to CSV/JSON for demo handoff.

## Context

Sprint 35 (E4) added the audit log panel. Sprint 38 adds a button to export the action log to
CSV/JSON for demo handoff.

## Acceptance criteria (from sprint)

1. Export produces valid CSV/JSON file.

## Test plan

### T1 — Export file format/content

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Populate the audit log with entries; trigger the export button.
- **Assertions:**
  - Export produces a file.
  - The file is valid CSV (or JSON) format.
  - The file contains all log entries with their fields (action, target, timestamp, result).
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e7_export` (or the equivalent Qt Test binary).

### T2 — Valid CSV/JSON

- **Type:** Qt unit test.
- **Assertions:**
  - For CSV: the file has a header and correctly formatted rows.
  - For JSON: the file is valid JSON with the log entries.
  - The exported content matches the log.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e7_valid` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt unit test.
- **Assertions:**
  - The audit log panel (E4) still works correctly.
  - Existing A1 and E4 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R e7_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Add a button to export the action log to CSV/JSON for demo handoff.
- Keep the existing log panel intact so A1/E4 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
