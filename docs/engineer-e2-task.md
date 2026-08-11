# Senior Engineer Task — Sprint 33 (S33 / E2 API action log endpoint)

You are **senior-engineer-e2**. Implement Sprint 33. Follow `docs/working-rules.md`.

## Sprint 33 (S33) — E2 API action log endpoint
- **Scope:** Add `GET /api/schedules/{id}/actions` returning the recorded operator actions.
- **Acceptance criteria:** Endpoint returns recorded actions; correct format.
- **Dependencies:** E1 (ActionLogEntry model, done), B3 (live control endpoints).

## Your job
1. Read `docs/phase-e2-tests.md` (test plan) and `docs/sprints-improvements.md` (S33).
2. Read `dotnet/PatchOrchestrator.Api/ActionLogEntry.cs` (E1), `dotnet/PatchOrchestrator.Api/
   EngineSession.cs` (B2), and `dotnet/PatchOrchestrator.Api/Program.cs` (B3 control
   endpoints, `ControlSession`, `Schedule` class, and the `schedules` store).
3. Add **action recording + a GET endpoint** in the .NET backend:
   - Extend `Schedule` (in `Program.cs`) with an `Actions` collection (e.g.
     `List<ActionLogEntry> Actions` or a `ConcurrentQueue`), initialized empty.
   - Record each operator action (schedule create, pause, resume, rollback — and optionally
     tick) as an `ActionLogEntry` (action, target = schedule id, timestamp = `DateTimeOffset.UtcNow`,
     result = resulting state) in `ControlSession` and in the schedule-create and tick paths.
   - Add `GET /api/schedules/{id}/actions` that returns the recorded actions in chronological
     order (the E1 type serializes with System.Text.Json). Return 404 for an unknown schedule
     id, matching the existing endpoints' style.
4. Add the E2 tests named in `docs/phase-e2-tests.md` in
   `dotnet/PatchOrchestrator.Api.Tests/` (new file `E2ApiTests.cs`), following the style of
   `B3ApiTests.cs` (WebApplicationFactory + isolated factory):
   - `E2_ReturnsActions` — after recording actions for a schedule, GET `/actions` returns the
     recorded count.
   - `E2_Format` — each returned action has correct fields (action, target, timestamp,
     result), chronological order, and GET on an unknown id returns 404.
   - `E2_Regression` — existing B3 and E1 tests still pass.
5. Build and run the tests **ONE AT A TIME with HARD TIMEOUTS**, e.g.
   `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "FullyQualifiedName~E2"`.
   Follow `docs/working-rules.md`: bounded `ls`/`grep`, never `find /`.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing backend behavior intact so existing B3/E1 and other tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
