# Phase D4 — Test Plan (Sprint 28: Scenario presets)

Engineer: `senior-engineer-d4`. Tester: `testing-d4`. This is the test plan for predefined
demo scenarios (small clean fleet, large fleet, high-failure fleet), each with fleet size,
failure rate, and seed.

## Context

Sprints 25–27 (D1–D3) added fleet size, failure rate, and seed controls (stored in
`DemoAppContext`). Sprint 28 defines predefined scenarios: small clean fleet, large fleet,
high-failure fleet — each with fleet size, failure rate, and seed.

## Acceptance criteria (from sprint)

1. Presets defined.
2. Each loads correct config values.

## Test plan

### T1 — Preset data correctness

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Load each predefined scenario preset.
- **Assertions:**
  - A preset exists for each scenario: small clean fleet, large fleet, high-failure fleet.
  - Each preset defines a valid fleet size, failure rate (0–1), and seed.
  - The values are internally consistent (e.g. small fleet is small, high-failure has a high
    failure rate).
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d4_presets` (or the equivalent Qt Test binary).

### T2 — Each loads correct config values

- **Type:** Qt unit test.
- **Assertions:**
  - Loading a preset populates the fleet size, failure rate, and seed correctly.
  - The loaded values match the preset definition.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d4_load` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt unit test.
- **Assertions:**
  - The `DemoAppContext` (A3) still works correctly.
  - Existing A3, B2, and D1–D3 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d4_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Define predefined demo scenarios (small clean fleet, large fleet, high-failure fleet), each
  with fleet size, failure rate, and seed.
- Keep the existing context and controls intact so A3/B2/D1–D3 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
