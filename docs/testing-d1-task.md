# Testing Agent Task — Sprint 25 (S25 / D1 Fleet size config)

You are **testing-d1**. Verify Sprint 25. Follow `docs/working-rules.md`.

## Sprint 25 (S25) — D1 Fleet size config
- **Scope:** UI control (spin box) to set number of endpoints in fleet before simulation.
- **Acceptance criteria:** Spin box sets fleet size; value stored in shared state.

## Your job
1. Read `docs/phase-d1-tests.md` (the test plan) and `docs/sprints-improvements.md` (S25).
2. Run the D1 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `d1_spinbox` (spin box sets fleet size)
   - `d1_control_context` (control updates context)
   - `d1_regression` (existing A3/B7 tests still pass)
   Use the VS 2022 dev environment for MSVC if needed. Use bounded `ls`/`grep`, never
   `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
