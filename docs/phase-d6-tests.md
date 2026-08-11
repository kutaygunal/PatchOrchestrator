# Phase D6 — Test Plan (Sprint 30: Config validation)

Engineer: `senior-engineer-d6`. Tester: `testing-d6`. This is the test plan for validating
fleet size (≥1), failure rate (0–1), and seed (int) before starting a rollout, with inline
errors.

## Context

Sprints 25–27 (D1–D3) added fleet size, failure rate, and seed controls. Sprint 30 validates
these before starting a rollout: fleet size ≥1, failure rate 0–1, seed is an integer; invalid
config is blocked with inline errors.

## Acceptance criteria (from sprint)

1. Invalid config blocked.
2. Inline errors shown.

## Test plan

### T1 — Validation rules

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Assertions:**
  - Fleet size < 1 is invalid; ≥1 is valid.
  - Failure rate < 0 or > 1 is invalid; 0–1 is valid.
  - Seed must be an integer (valid).
  - A valid config passes validation.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d6_rules` (or the equivalent Qt Test binary).

### T2 — Invalid config blocked

- **Type:** Qt unit test.
- **Assertions:**
  - Starting a rollout with invalid config is blocked.
  - No rollout starts with invalid values.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d6_blocked` (or the equivalent Qt Test binary).

### T3 — Inline errors shown

- **Type:** Qt widget test.
- **Assertions:**
  - Invalid config shows an inline error near the relevant control.
  - The error message identifies the problem (e.g. fleet size must be ≥1, failure rate must be
    0–1, seed must be an integer).
  - The error clears when the value is corrected.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d6_errors` (or the equivalent Qt Test binary).

### T4 — Regression

- **Type:** Qt unit test.
- **Assertions:**
  - The config controls (D1–D3) still work correctly.
  - Existing A3, B2, and D1–D5 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d6_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Validate fleet size (≥1), failure rate (0–1), and seed (int) before starting a rollout;
  block invalid config and show inline errors.
- Keep the existing config controls intact so A3/B2/D1–D5 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
