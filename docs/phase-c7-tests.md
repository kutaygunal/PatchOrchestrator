# Phase C7 — Test Plan (Sprint 24: Dashboard legend)

Engineer: `senior-engineer-c7`. Tester: `testing-c7`. This is the test plan for a legend in
the dashboard that explains the color coding and state meanings for demo viewers.

## Context

Sprint 19 (C2) added the state→color mapping (green=succeeded, red=failed, amber=paused,
blue=running, grey=pending, purple=rolled_back). Sprint 24 adds a legend to the dashboard
that explains the color coding and state meanings for demo viewers.

## Acceptance criteria (from sprint)

1. Legend visible.
2. Explains all states/colors.

## Test plan

### T1 — Legend renders

- **Type:** Qt widget test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct the dashboard (or legend widget) with the C2 state→color mapping.
- **Assertions:**
  - The legend is visible in the dashboard.
  - The legend renders without crashing.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c7_legend_renders` (or the equivalent Qt Test binary).

### T2 — Explains all states/colors

- **Type:** Qt widget test.
- **Assertions:**
  - The legend includes all six states: succeeded, failed, paused, running, pending,
    rolled_back.
  - Each state entry shows the correct color (from the C2 mapping).
  - Each state entry shows its meaning/label.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c7_all_states` (or the equivalent Qt Test binary).

### T3 — Regression

- **Type:** Qt widget test.
- **Assertions:**
  - The dashboard still renders endpoint data correctly with the legend.
  - Existing dashboard tests (P8) and C2 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c7_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Add a legend to the dashboard explaining the color coding and state meanings, using the C2
  mapping.
- Keep the existing dashboard behavior and C2 mapping intact so P8/C2 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
