# Testing Agent Task — Sprint 34 (S34 / E3 Action recording)

You are **testing-e3**. Verify Sprint 34. Follow `docs/working-rules.md`.

## Sprint 34 (S34) — E3 Action recording
- **Scope:** Record schedule/pause/resume/rollback actions (with timestamps) in the
  `EngineSession`/API.
- **Acceptance criteria:** Actions recorded with timestamps; complete log.

## Your job
1. Read `docs/phase-e3-tests.md` (the test plan) and `docs/sprints-improvements.md` (S34).
2. Run the E3 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `E3_Recorded` (actions recorded on operations with timestamps)
   - `E3_CompleteLog` (complete log, count matches, in order)
   - `E3_Regression` (existing B3, E1, E2 tests still pass)
   Run them with `dotnet test dotnet/PatchOrchestrator.Api.Tests --filter ...`. Use bounded
   `ls`/`grep`, never `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
