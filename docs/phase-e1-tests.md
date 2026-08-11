# Phase E1 — Test Plan (Sprint 32: Action log model)

Engineer: `senior-engineer-e1`. Tester: `testing-e1`. This is the test plan for the
C++/server-side `ActionLogEntry` type (action, target, timestamp, result).

## Context

Sprint 3 (A3) added `DemoAppContext` (shared state). Sprint 32 adds an `ActionLogEntry` type
(action, target, timestamp, result) used to record operator actions (schedule, pause, resume,
rollback).

## Acceptance criteria (from sprint)

1. Type defined with all fields.
2. Serializable.

## Test plan

### T1 — Field access

- **Type:** .NET/C++ unit test (xUnit or Qt, bounded with timeout).
- **Assertions:**
  - The `ActionLogEntry` type has the fields: action, target, timestamp, result.
  - Each field can be set and retrieved correctly.
  - The fields are populated with the correct values.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E1_FieldAccess"` (or
  the equivalent Qt Test binary).

### T2 — Serialization

- **Type:** .NET/C++ unit test.
- **Assertions:**
  - An `ActionLogEntry` serializes to the correct format (e.g. JSON).
  - All fields are preserved through serialization/deserialization.
  - The serialized form is valid.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E1_Serialization"` (or
  the equivalent Qt Test binary).

### T3 — Regression

- **Type:** .NET/C++ unit test.
- **Assertions:**
  - The `DemoAppContext` (A3) still works correctly.
  - Existing A3 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "E1_Regression"` (or
  the equivalent Qt Test binary).

## Notes for the engineer

- Add an `ActionLogEntry` type (action, target, timestamp, result) that can be serialized.
- Keep the existing context intact so A3 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
