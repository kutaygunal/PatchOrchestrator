# Phase D7 — Test Plan (Sprint 31: Config → engine wiring)

Engineer: `senior-engineer-d7`. Tester: `testing-d7`. This is the test plan for passing the
configured fleet size, failure rate, and seed into the live `EngineSession` when starting a
rollout.

## Context

Sprints 25–27 (D1–D3) added fleet size, failure rate, and seed controls (stored in
`DemoAppContext`). Sprint 30 (D6) added config validation. Sprint 31 wires the configured
fleet size, failure rate, and seed into the live `EngineSession` (B2) when starting a rollout.

## Acceptance criteria (from sprint)

1. Config passed to session.
2. Rollout uses configured values.

## Test plan

### T1 — Config reaches engine

- **Type:** .NET integration test (xUnit, bounded with timeout).
- **Setup:** Configure fleet size, failure rate, and seed; start a rollout via the live
  `EngineSession`.
- **Assertions:**
  - The session receives the configured fleet size, failure rate, and seed.
  - The rollout is created with the configured values.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "D7_ConfigReachesEngine"`

### T2 — Rollout uses configured values

- **Type:** .NET integration test.
- **Assertions:**
  - The rollout has the configured number of endpoints.
  - Endpoints use the configured failure rate.
  - The rollout is deterministic for the configured seed.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "D7_RolloutUsesConfig"`

### T3 — Regression

- **Type:** .NET integration test.
- **Assertions:**
  - The live `EngineSession` (B2) still works correctly.
  - Existing B2 and D1–D6 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "D7_Regression"`

## Notes for the engineer

- Pass the configured fleet size, failure rate, and seed into the live `EngineSession` (B2)
  when starting a rollout.
- Keep the existing session and config logic intact so B2/D1–D6 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
