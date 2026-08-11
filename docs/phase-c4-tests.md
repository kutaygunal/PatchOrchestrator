# Phase C4 — Test Plan (Sprint 21: Fleet summary panel)

Engineer: `senior-engineer-c4`. Tester: `testing-c4`. This is the test plan for a summary
widget showing counts by state (succeeded/failed/paused/running/pending/rolled_back) and total.

## Context

Sprint 19 (C2) added the state→color mapping. Sprint 20 (C3) added the reusable `StateBadge`.
Sprint 21 adds a fleet summary panel that aggregates endpoint counts by state and shows the
total.

## Acceptance criteria (from sprint)

1. Counts correct per state.
2. Total correct.
3. Updates on change.

## Test plan

### T1 — Count aggregation

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Provide a set of endpoints with known states.
- **Assertions:**
  - The count for each state (succeeded/failed/paused/running/pending/rolled_back) matches the
    number of endpoints in that state.
  - The total equals the sum of all per-state counts (and the total number of endpoints).
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c4_aggregation` (or the equivalent Qt Test binary).

### T2 — Total correct

- **Type:** Qt unit test.
- **Assertions:**
  - The total equals the number of endpoints.
  - The total equals the sum of the six per-state counts.
  - An empty fleet shows zero counts and a total of zero.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c4_total` (or the equivalent Qt Test binary).

### T3 — Updates on change

- **Type:** Qt unit test.
- **Assertions:**
  - When an endpoint's state changes, the summary counts update accordingly.
  - Adding/removing endpoints updates the counts and total.
  - The summary reflects the latest data.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c4_updates` (or the equivalent Qt Test binary).

### T4 — Regression

- **Type:** Qt widget test.
- **Assertions:**
  - The dashboard still renders endpoint data correctly with the summary panel.
  - Existing dashboard tests (P8), C1, C2, and C3 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c4_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Add a fleet summary panel that aggregates endpoint counts by state and shows the total.
- Use the C2 state set and C3 `StateBadge` for consistent rendering.
- Keep the existing dashboard behavior intact so P8/C1/C2/C3 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
