# Phase D5 — Test Plan (Sprint 29: Scenario selector)

Engineer: `senior-engineer-d5`. Tester: `testing-d5`. This is the test plan for a
dropdown/button group that loads a preset scenario into the config controls.

## Context

Sprint 28 (D4) added predefined scenario presets (small clean fleet, large fleet,
high-failure fleet). Sprint 29 adds a dropdown/button group to load a preset scenario into
the config controls (fleet size, failure rate, seed).

## Acceptance criteria (from sprint)

1. Selecting preset populates controls.
2. Overrides manual values.

## Test plan

### T1 — Preset loads into controls

- **Type:** Qt widget test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct the scenario selector wired to the config controls.
- **Assertions:**
  - Selecting a preset loads its fleet size, failure rate, and seed into the controls.
  - The controls reflect the preset values.
  - All presets are selectable.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d5_preset_load` (or the equivalent Qt Test binary).

### T2 — Overrides manual values

- **Type:** Qt widget test.
- **Assertions:**
  - Setting manual values, then selecting a preset, overrides the manual values with the
    preset's values.
  - The controls reflect the preset values (not the manual ones).
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d5_override` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt widget test.
- **Assertions:**
  - The scenario presets (D4) still work correctly.
  - Existing A3, B2, and D1–D4 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R d5_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Add a dropdown/button group to load a preset scenario into the config controls, overriding
  manual values.
- Keep the existing presets and controls intact so A3/B2/D1–D4 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
