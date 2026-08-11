# Senior Engineer Task — Sprint 34 (S34 / E3 Action recording)

You are **senior-engineer-e3**. Implement Sprint 34. Follow `docs/working-rules.md`.

## Sprint 34 (S34) — E3 Action recording
- **Scope:** Record schedule/pause/resume/rollback actions (with timestamps) in the
  `EngineSession`/API.
- **Acceptance criteria:** Actions recorded with timestamps; complete log.
- **Dependencies:** E1 (ActionLogEntry), E2 (action log endpoint), B3 (control endpoints).

## Your job
1. Read `docs/phase-e3-tests.md` (test plan) and `docs/sprints-improvements.md` (S34).
2. Read `dotnet/PatchOrchestrator.Api/ActionLogEntry.cs` (E1) and
   `dotnet/PatchOrchestrator.Api/Program.cs` — Sprint 33 (E2) added action recording to the
   `Schedule` `Actions` collection and the `GET /api/schedules/{id}/actions` endpoint.
3. Complete and harden **action recording** in the .NET backend so every operator action is
   recorded with a timestamp:
   - Ensure **schedule create**, **pause**, **resume**, and **rollback** each append an
     `ActionLogEntry` (action, target = schedule id, timestamp = `DateTimeOffset.UtcNow`,
     result = resulting state) to the schedule's `Actions` collection. Add any missing
     recording (e.g. tick) so the log is complete.
   - Ensure each recorded entry has a valid timestamp and the correct action type/result, and
     that the log preserves chronological order. The E2 `GET /actions` endpoint already
     returns them; keep it working.
4. Add the E3 tests named in `docs/phase-e3-tests.md` in
   `dotnet/PatchOrchestrator.Api.Tests/` (new file `E3ActionRecordingTests.cs`), following the
   style of `E2ApiTests.cs` / `B3ApiTests.cs`:
   - `E3_Recorded` — each schedule/pause/resume/rollback operation records an action entry
     with correct type, target, result, and a timestamp.
   - `E3_CompleteLog` — the log contains all performed operations in order, count matches.
   - `E3_Regression` — existing B3, E1, and E2 tests still pass.
5. Build and run the tests **ONE AT A TIME with HARD TIMEOUTS**, e.g.
   `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "FullyQualifiedName~E3"`.
   Follow `docs/working-rules.md`: bounded `ls`/`grep`, never `find /`.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing backend behavior intact so existing B3/E1/E2 and other tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
