# Senior Engineer Task — Sprint 32 (S32 / E1 Action log model)

You are **senior-engineer-e1**. Implement Sprint 32. Follow `docs/working-rules.md`.

## Sprint 32 (S32) — E1 Action log model
- **Scope:** A server-side `ActionLogEntry` type (action, target, timestamp, result) used to
  record operator actions (schedule, pause, resume, rollback).
- **Acceptance criteria:** Type defined with all fields; serializable.
- **Dependencies:** A3 (shared context).

## Your job
1. Read `docs/phase-e1-tests.md` (test plan) and `docs/sprints-improvements.md` (S32).
2. Understand the .NET backend types in `dotnet/PatchOrchestrator.Api/` — the existing
   `record` types (e.g. `EngineRequest`, `EngineResult`, `CreateScheduleRequest`) use C#
   positional records. The action log type should follow the same style so it serializes
   cleanly with System.Text.Json.
3. Add a new **ActionLogEntry** type in the .NET backend (e.g. `dotnet/PatchOrchestrator.Api/
   ActionLogEntry.cs`), a C# positional `record` with the fields:
   - `Action` (string — e.g. "schedule", "pause", "resume", "rollback"),
   - `Target` (string — the schedule/endpoint id),
   - `Timestamp` (DateTimeOffset — when the action occurred, ISO-8601),
   - `Result` (string — e.g. "ok" / "error", or the resulting state).
   Use a public positional record (or a class with an object initializer) that round-trips
   through System.Text.Json (fields preserved on serialize/deserialize).
4. Add the E1 tests named in `docs/phase-e1-tests.md` in
   `dotnet/PatchOrchestrator.Api.Tests/` (new file `E1ActionLogModelTests.cs`), following the
   style of the existing test files:
   - `E1_FieldAccess` — the type has action/target/timestamp/result fields, each set and
     retrieved correctly.
   - `E1_Serialization` — an ActionLogEntry serializes to JSON with all fields preserved
     through round-trip.
   - `E1_Regression` — existing A3 / backend tests still pass.
5. Build and run the tests **ONE AT A TIME with HARD TIMEOUTS**, e.g.
   `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "FullyQualifiedName~E1"`.
   Follow `docs/working-rules.md`: bounded `ls`/`grep`, never `find /`.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing backend behavior intact so existing tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
