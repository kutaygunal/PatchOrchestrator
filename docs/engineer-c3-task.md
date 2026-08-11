# Senior Engineer Task — Sprint 20 (S20 / C3 State badge renderer)

You are **senior-engineer-c3**. Implement Sprint 20. Follow `docs/working-rules.md`.

## Sprint 20 (S20) — C3 State badge renderer
- **Scope:** Reusable `StateBadge` widget/icon rendering color-coded state with legend.
- **Acceptance criteria:** Badge renders correct color/icon; reusable; legend present.
- **Dependencies:** C2 (state→color mapping, done).

## Your job
1. Read `docs/phase-c3-tests.md` (test plan) and `docs/sprints-improvements.md` (S20).
2. Read the dashboard (`src/ui/dashboard.cpp` / `.hpp`) and the C2 state→color mapping.
3. Extract a reusable `StateBadge` widget/icon that renders the color-coded state (using the
   C2 mapping) and includes a legend. Use the badge in the dashboard in place of inline color
   coding.
4. Write the C3 tests named in the test plan (`c3_badge_render`, `c3_reusable`, `c3_legend`,
   `c3_regression`).
5. Build with a HARD TIMEOUT and run the tests ONE AT A TIME. Use the VS 2022 dev
   environment for MSVC if needed.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do NOT commit or push.
- Run build/test commands with a hard timeout.
- Keep the existing dashboard behavior and C2 mapping intact so P8/C1/C2 tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
