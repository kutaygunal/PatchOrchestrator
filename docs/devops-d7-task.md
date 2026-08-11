# Devops Agent Task — Sprint 31 (S31 / D7 Config → engine wiring)

You are **devops-d7**. Commit and push Sprint 31. Follow `docs/working-rules.md`.

## Sprint 31 (S31) — D7 Config → engine wiring
- **Scope:** Pass the configured fleet size, failure rate, and seed into the live
  `EngineSession` (B2) when starting a rollout.
- **Status:** Engineer implemented; testing-d7 reported PASS.

## Your job
1. Verify the working tree contains the D7 changes:
   - `dotnet/PatchOrchestrator.Api/EngineRequestFactory.cs` (new config→request builder)
   - `dotnet/PatchOrchestrator.Api/Program.cs` (wired the factory when creating a session)
   - `dotnet/PatchOrchestrator.Api.Tests/D7ConfigWiringTests.cs` (new D7 tests)
   - `docs/phase-d7-tests.md`, `docs/engineer-d7-task.md`, `docs/testing-d7-task.md`
2. Stage and commit the D7 implementation + tests with a conventional message, e.g.
   `feat(api): config to engine wiring (D7)`.
3. Update `docs/sprints-improvements.md` S31 Status to "DONE" and commit as
   `docs: mark S31 (D7) as DONE`.
4. Push to origin if a remote exists; otherwise note that the repo has no remote and the
   commit is local-only.

## Constraints
- Use bounded `ls`/`grep`, never `find /`.
- Do NOT modify source code or tests.
- Run git commands with a hard timeout.

## Reply
Reply with a **single sentence**: "Pushed <commit-hash> — S31 (D7) committed and pushed." or
"Committed <hash> locally; no remote configured."
