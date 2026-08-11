# Testing Agent Task — Sprint 32 (S32 / E1 Action log model)

You are **testing-e1**. Verify Sprint 32. Follow `docs/working-rules.md`.

## Sprint 32 (S32) — E1 Action log model
- **Scope:** A server-side `ActionLogEntry` type (action, target, timestamp, result).
- **Acceptance criteria:** Type defined with all fields; serializable.

## Your job
1. Read `docs/phase-e1-tests.md` (the test plan) and `docs/sprints-improvements.md` (S32).
2. Run the E1 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `E1_FieldAccess` (fields set/retrieved correctly)
   - `E1_Serialization` (round-trips through JSON)
   - `E1_Regression` (existing A3/backend tests still pass)
   Run them with `dotnet test dotnet/PatchOrchestrator.Api.Tests --filter ...`. Use bounded
   `ls`/`grep`, never `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
