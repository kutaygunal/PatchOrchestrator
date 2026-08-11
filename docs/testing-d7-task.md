# Testing Agent Task — Sprint 31 (S31 / D7 Config → engine wiring)

You are **testing-d7**. Verify Sprint 31. Follow `docs/working-rules.md`.

## Sprint 31 (S31) — D7 Config → engine wiring
- **Scope:** Pass the configured fleet size, failure rate, and seed into the live
  `EngineSession` (B2) when starting a rollout.
- **Acceptance criteria:** Config passed to session; rollout uses configured values.

## Your job
1. Read `docs/phase-d7-tests.md` (the test plan) and `docs/sprints-improvements.md` (S31).
2. Run the D7 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `D7_ConfigReachesEngine` (config reaches engine)
   - `D7_RolloutUsesConfig` (rollout uses configured values)
   - `D7_Regression` (existing B2 and D1–D6 tests still pass)
   Run them with `dotnet test dotnet/PatchOrchestrator.Api.Tests --filter ...`. Use bounded
   `ls`/`grep`, never `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
