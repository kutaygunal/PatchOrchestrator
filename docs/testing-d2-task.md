# Testing Agent Task — Sprint 26 (S26 / D2 Failure rate config)

You are **testing-d2**. Verify Sprint 26. Follow `docs/working-rules.md`.

## Sprint 26 (S26) — D2 Failure rate config
- **Scope:** UI control (slider/spin box) to set per-endpoint failure rate (0.0–1.0).
- **Acceptance criteria:** Control sets failure rate; value stored in shared state.

## Your job
1. Read `docs/phase-d2-tests.md` (the test plan) and `docs/sprints-improvements.md` (S26).
2. Run the D2 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `d2_range` (failure rate range 0.0–1.0)
   - `d2_control_context` (control updates context)
   - `d2_regression` (existing tests still pass)
   Use the VS 2022 dev environment for MSVC if needed. Use bounded `ls`/`grep`, never
   `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
