# Testing Agent Task — Sprint 20 (S20 / C3 State badge renderer)

You are **testing-c3**. Verify Sprint 20. Follow `docs/working-rules.md`.

## Sprint 20 (S20) — C3 State badge renderer
- **Scope:** Reusable `StateBadge` widget/icon rendering color-coded state with legend.
- **Acceptance criteria:** Badge renders correct color/icon; reusable; legend present.

## Your job
1. Read `docs/phase-c3-tests.md` (the test plan) and `docs/sprints-improvements.md` (S20).
2. Run the C3 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `c3_badge_render` (badge renders correct color/icon per state)
   - `c3_reusable` (reusable, multiple badges, state update)
   - `c3_legend` (legend present with all states)
   - `c3_regression` (dashboard still renders; P8/C1/C2 tests still pass)
   Use the VS 2022 dev environment for MSVC if needed. Use bounded `ls`/`grep`, never
   `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
