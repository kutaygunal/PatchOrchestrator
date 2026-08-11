# Testing Agent Task — Sprint 24 (S24 / C7 Dashboard legend)

You are **testing-c7**. Verify Sprint 24. Follow `docs/working-rules.md`.

## Sprint 24 (S24) — C7 Dashboard legend
- **Scope:** Legend explaining color coding and state meanings for demo viewers.
- **Acceptance criteria:** Legend visible; explains all states/colors.

## Your job
1. Read `docs/phase-c7-tests.md` (the test plan) and `docs/sprints-improvements.md` (S24).
2. Run the C7 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `c7_legend_renders` (legend renders)
   - `c7_all_states` (explains all states/colors)
   - `c7_regression` (dashboard still renders; P8/C1/C2/C3/C4/C5/C6 tests still pass)
   Use the VS 2022 dev environment for MSVC if needed. Use bounded `ls`/`grep`, never
   `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
