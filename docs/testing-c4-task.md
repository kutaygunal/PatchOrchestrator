# Testing Agent Task — Sprint 21 (S21 / C4 Fleet summary panel)

You are **testing-c4**. Verify Sprint 21. Follow `docs/working-rules.md`.

## Sprint 21 (S21) — C4 Fleet summary panel
- **Scope:** Summary widget showing counts by state (succeeded/failed/paused/running/
  pending/rolled_back) and total.
- **Acceptance criteria:** Counts correct per state; total correct; updates on change.

## Your job
1. Read `docs/phase-c4-tests.md` (the test plan) and `docs/sprints-improvements.md` (S21).
2. Run the C4 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `c4_aggregation` (counts by state)
   - `c4_total` (total correct)
   - `c4_updates` (updates on change)
   - `c4_regression` (dashboard still renders; P8/C1/C2/C3 tests still pass)
   Use the VS 2022 dev environment for MSVC if needed. Use bounded `ls`/`grep`, never
   `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
