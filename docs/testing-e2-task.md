# Testing Agent Task — Sprint 33 (S33 / E2 API action log endpoint)

You are **testing-e2**. Verify Sprint 33. Follow `docs/working-rules.md`.

## Sprint 33 (S33) — E2 API action log endpoint
- **Scope:** Add `GET /api/schedules/{id}/actions` returning the recorded operator actions.
- **Acceptance criteria:** Endpoint returns recorded actions; correct format.

## Your job
1. Read `docs/phase-e2-tests.md` (the test plan) and `docs/sprints-improvements.md` (S33).
2. Run the E2 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `E2_ReturnsActions` (endpoint returns recorded actions)
   - `E2_Format` (correct fields/order, 404 on unknown id)
   - `E2_Regression` (existing B3 and E1 tests still pass)
   Run them with `dotnet test dotnet/PatchOrchestrator.Api.Tests --filter ...`. Use bounded
   `ls`/`grep`, never `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
