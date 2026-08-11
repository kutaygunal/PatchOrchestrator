# Phase D2 — Test Plan (Sprint 26: Failure rate config)

Engineer: `senior-engineer-d2`. Tester: `testing-d2`. This is the test plan for a UI control
(slider/spin box) that sets the per-endpoint failure rate (0.0–1.0).

## Context

Sprint 3 (A3) added `DemoAppContext` (shared state). Sprint 12 (B2) added the live
`EngineSession`. Sprint 26 adds a failure-rate control (slider/spin box) that sets the
per-endpoint failure rate (0.0–1.0), storing the value in the shared context.

## Acceptance criteria (from sprint)

1. Control sets failure rate.
2. Value stored in shared state.

## Test plan

### T1 — Control updates context

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct the failure-rate control wired to a `DemoAppContext`.
- **Assertions:**
  - Changing the control value updates the context's failure rate.
  - The context reflects the new value.
  - The control reads the initial value from the context.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d2_control_context` (or the equivalent Qt Test binary).

### T2 — Control sets failure rate (0.0–1.0)

- **Type:** Qt unit test.
- **Assertions:**
  - The control has a range of 0.0–1.0.
  - Setting the control to a value updates the failure rate.
  - The value is stored in the shared context.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d2_range` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt unit test.
- **Assertions:**
  - The `DemoAppContext` (A3) still works correctly.
  - Existing A3, B2, and D1 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d2_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Add a failure-rate control (slider/spin box) that sets the per-endpoint failure rate
  (0.0–1.0), storing the value in the shared `DemoAppContext` (A3).
- Keep the existing context and session intact so A3/B2/D1 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
