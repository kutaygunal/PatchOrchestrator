# Phase D1 — Test Plan (Sprint 25: Fleet size config)

Engineer: `senior-engineer-d1`. Tester: `testing-d1`. This is the test plan for a UI control
(spin box) that sets the number of endpoints in the fleet before simulation.

## Context

Sprint 3 (A3) added `DemoAppContext` (shared state). Sprint 12 (B2) added the live
`EngineSession`. Sprint 25 adds a fleet-size spin box control that sets the number of
endpoints in the fleet before simulation, storing the value in the shared context.

## Acceptance criteria (from sprint)

1. Spin box sets fleet size.
2. Value stored in shared state.

## Test plan

### T1 — Control updates context

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct the fleet-size control wired to a `DemoAppContext`.
- **Assertions:**
  - Changing the spin box value updates the context's fleet size.
  - The context reflects the new value.
  - The control reads the initial value from the context.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d1_control_context` (or the equivalent Qt Test binary).

### T2 — Spin box sets fleet size

- **Type:** Qt unit test.
- **Assertions:**
  - The spin box has a sensible range (e.g. ≥1).
  - Setting the spin box to a value updates the fleet size.
  - The value is stored in the shared context.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d1_spinbox` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt unit test.
- **Assertions:**
  - The `DemoAppContext` (A3) still works correctly.
  - Existing A3 and B2 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d1_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Add a fleet-size spin box control that sets the number of endpoints, storing the value in
  the shared `DemoAppContext` (A3).
- Keep the existing context and session intact so A3/B2 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
