# Phase C3 — Test Plan (Sprint 20: State badge renderer)

Engineer: `senior-engineer-c3`. Tester: `testing-c3`. This is the test plan for a reusable
`StateBadge` widget/icon that renders the color-coded state with a legend.

## Context

Sprint 19 (C2) added a state→color mapping (green=succeeded, red=failed, amber=paused,
blue=running, grey=pending, purple=rolled_back) applied to rows/badges in the dashboard.
Sprint 20 extracts a reusable `StateBadge` widget/icon that renders the color-coded state and
includes a legend.

## Acceptance criteria (from sprint)

1. Badge renders correct color/icon.
2. Reusable.
3. Legend present.

## Test plan

### T1 — Badge renders correct color/icon per state

- **Type:** Qt widget test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct a `StateBadge` for each patch state.
- **Assertions:**
  - Each state renders the correct color (from the C2 mapping).
  - Each state renders the correct icon/label.
  - The badge reflects the state it is given.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c3_badge_render` (or the equivalent Qt Test binary).

### T2 — Reusable

- **Type:** Qt widget test.
- **Assertions:**
  - The `StateBadge` can be constructed standalone and embedded in other widgets.
  - Multiple badges can coexist with different states.
  - Updating a badge's state re-renders it correctly.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c3_reusable` (or the equivalent Qt Test binary).

### T3 — Legend present

- **Type:** Qt widget test.
- **Assertions:**
  - The badge (or its legend) explains the color coding and state meanings.
  - All six states appear in the legend with their color and meaning.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c3_legend` (or the equivalent Qt Test binary).

### T4 — Regression

- **Type:** Qt widget test.
- **Assertions:**
  - The dashboard still renders endpoint data correctly using the badge.
  - Existing dashboard tests (P8), C1, and C2 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c3_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Extract a reusable `StateBadge` widget/icon that renders the color-coded state (using the C2
  mapping) and includes a legend.
- Use the badge in the dashboard in place of inline color coding.
- Keep the existing dashboard behavior and C2 mapping intact so P8/C1/C2 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
