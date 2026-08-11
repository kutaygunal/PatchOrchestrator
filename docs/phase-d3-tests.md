# Phase D3 — Test Plan (Sprint 27: Seed config)

Engineer: `senior-engineer-d3`. Tester: `testing-d3`. This is the test plan for a UI control
that sets the deterministic seed for reproducible demos.

## Context

Sprint 3 (A3) added `DemoAppContext` (shared state). Sprint 12 (B2) added the live
`EngineSession` (deterministic/seeded). Sprint 27 adds a seed control that sets the
deterministic seed for reproducible demos, storing the value in the shared context.

## Acceptance criteria (from sprint)

1. Control sets seed.
2. Value stored in shared state.

## Test plan

### T1 — Control updates context

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct the seed control wired to a `DemoAppContext`.
- **Assertions:**
  - Changing the control value updates the context's seed.
  - The context reflects the new value.
  - The control reads the initial value from the context.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d3_control_context` (or the equivalent Qt Test binary).

### T2 — Control sets seed (integer)

- **Type:** Qt unit test.
- **Assertions:**
  - The control accepts an integer seed.
  - Setting the control to a value updates the seed.
  - The value is stored in the shared context.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d3_seed` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt unit test.
- **Assertions:**
  - The `DemoAppContext` (A3) still works correctly.
  - Existing A3, B2, D1, and D2 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d3_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Add a seed control that sets the deterministic seed, storing the value in the shared
  `DemoAppContext` (A3).
- Keep the existing context and session intact so A3/B2/D1/D2 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
