# Phase E3 — Test Plan (Sprint 34: Action recording)

Engineer: `senior-engineer-e3`. Tester: `testing-e3`. This is the test plan for recording
schedule/pause/resume/rollback actions (with timestamps) in the `EngineSession`/API.

## Context

Sprint 32 (E1) added the `ActionLogEntry` type. Sprint 33 (E2) added the action log endpoint.
Sprint 34 records schedule/pause/resume/rollback actions (with timestamps) in the
`EngineSession`/API.

## Acceptance criteria (from sprint)

1. Actions recorded with timestamps.
2. Complete log.

## Test plan

### T1 — Actions recorded on operations

- **Type:** .NET unit test (xUnit, bounded with timeout).
- **Setup:** Perform schedule/pause/resume/rollback operations.
- **Assertions:**
  - Each operation records an action entry.
  - Each entry has the correct action type (schedule/pause/resume/rollback), target, and result.
  - Each entry has a timestamp.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E3_Recorded"`

### T2 — Complete log

- **Type:** .NET unit test.
- **Assertions:**
  - The log contains all performed operations in order.
  - The number of entries matches the number of operations.
  - No operations are missing from the log.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E3_CompleteLog"`

### T3 — Regression

- **Type:** .NET unit test.
- **Assertions:**
  - The live control endpoints (B3) still work correctly.
  - Existing B3, E1, and E2 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E3_Regression"`

## Notes for the engineer

- Record schedule/pause/resume/rollback actions (with timestamps) in the `EngineSession`/API
  using the E1 `ActionLogEntry` type.
- Keep the existing endpoints and model intact so B3/E1/E2 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
