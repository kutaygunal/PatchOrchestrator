# Testing Agent Task — Sprint 22 (S22 / C5 Rollout-stage grouping)

You are **testing-c5**. Verify Sprint 22. Follow `docs/working-rules.md`.

## Sprint 22 (S22) — C5 Rollout-stage grouping
- **Scope:** Group endpoints by rollout stage (wave/group) with stage headers and per-stage
  progress.
- **Acceptance criteria:** Endpoints grouped by stage; headers + per-stage progress shown.

## Your job
1. Read `docs/phase-c5-tests.md` (the test plan) and `docs/sprints-improvements.md` (S22).
2. Run the C5 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `c5_grouping` (grouping logic)
   - `c5_headers` (stage headers)
   - `c5_progress` (per-stage progress)
   - `c5_regression` (dashboard still renders; P8/C1/C2/C3/C4 tests still pass)
   Use the VS 2022 dev environment for MSVC if needed. Use bounded `ls`/`grep`, never
   `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
