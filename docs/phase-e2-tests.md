# Phase E2 — Test Plan (Sprint 33: API action log endpoint)

Engineer: `senior-engineer-e2`. Tester: `testing-e2`. This is the test plan for adding
`GET /api/schedules/{id}/actions` that returns the recorded operator actions.

## Context

Sprint 32 (E1) added the `ActionLogEntry` type. Sprint 33 adds `GET /api/schedules/{id}/actions`
returning the recorded operator actions.

## Acceptance criteria (from sprint)

1. Endpoint returns recorded actions.
2. Correct format.

## Test plan

### T1 — Endpoint returns recorded actions

- **Type:** .NET integration test (xUnit + WebApplicationFactory, bounded with timeout).
- **Setup:** Record some operator actions for a schedule, then GET `/api/schedules/{id}/actions`.
- **Assertions:**
  - The endpoint returns the recorded actions.
  - The number of returned actions matches the recorded count.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E2_ReturnsActions"`

### T2 — Correct format

- **Type:** .NET integration test.
- **Assertions:**
  - Each returned action has the correct fields (action, target, timestamp, result).
  - The actions are in the correct order (e.g. chronological).
  - GET on an unknown schedule id returns 404 Not Found.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E2_Format"`

### T3 — Regression

- **Type:** .NET integration test.
- **Assertions:**
  - Existing endpoints (pause/resume/rollback/status/simulate) still work.
  - Existing B3 and E1 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E2_Regression"`

## Notes for the engineer

- Add `GET /api/schedules/{id}/actions` returning the recorded operator actions using the E1
  `ActionLogEntry` type.
- Keep the existing endpoints and model intact so B3/E1 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
