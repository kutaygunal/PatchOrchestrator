# Senior Engineer Task — Sprint 31 (S31 / D7 Config → engine wiring)

You are **senior-engineer-d7**. Implement Sprint 31. Follow `docs/working-rules.md`.

## Sprint 31 (S31) — D7 Config → engine wiring
- **Scope:** Pass the configured fleet size, failure rate, and seed into the live
  `EngineSession` (B2) when starting a rollout.
- **Acceptance criteria:** Config passed to session; rollout uses configured values.
- **Dependencies:** D1–D3 (config controls), D6 (validation), B2 (EngineSession).

## Your job
1. Read `docs/phase-d7-tests.md` (test plan) and `docs/sprints-improvements.md` (S31).
2. Understand the .NET backend:
   - `dotnet/PatchOrchestrator.Api/EngineBridge.cs` — `EngineRequest(IReadOnlyList<EngineEndpointRequest> Endpoints, int Seed)` and `EngineEndpointRequest(Id, FailureRate)`.
   - `dotnet/PatchOrchestrator.Api/EngineSession.cs` — B2 session; ctor takes
     `(IEngineBridge bridge, EngineRequest request)` and starts a live rollout.
   - `dotnet/PatchOrchestrator.Api/Program.cs` — API endpoints; the simulate endpoint builds
     `EngineRequest` from `SimulateRequest`. Review `CreateDefaultRequest()`.
3. Add a **config → engine wiring helper** in the .NET backend that turns a configured fleet
   size, failure rate, and seed into an `EngineRequest` for `EngineSession`. A clean,
   testable approach: add a static builder (e.g. `EngineRequestFactory.Build(int fleetSize,
   double failureRate, int seed)`) that generates `fleetSize` endpoints (ids like `ep-1`
   … `ep-N`) each with the given `FailureRate` and the given `Seed`. Place it in
   `dotnet/PatchOrchestrator.Api/` (new file `EngineRequestFactory.cs`). Use it when creating
   an `EngineSession` from configured values.
4. Wire this so a rollout started from configured D1–D3 values uses those values (endpoints
   count = fleet size, each endpoint's failure rate, and the seed). Ensure an invalid config
   (fleet size < 1, failure rate outside 0–1) is guarded/clamped or rejected before building
   the request.
5. Add the D7 tests named in `docs/phase-d7-tests.md` in
   `dotnet/PatchOrchestrator.Api.Tests/` (new file `D7ConfigWiringTests.cs`), following the
   style of the existing `EngineSessionTests.cs` and `B3ApiTests.cs`:
   - `D7_ConfigReachesEngine` — building a request from config passes the configured fleet
     size, failure rate, and seed to the session/request.
   - `D7_RolloutUsesConfig` — the rollout has the configured number of endpoints, endpoints
     use the configured failure rate, and it is deterministic for the seed.
   - `D7_Regression` — the live EngineSession still works; existing B2 tests still pass.
6. Build and run the tests **ONE AT A TIME with HARD TIMEOUTS**, e.g.
   `timeout 300 dotnet test dotnet/PatchOrchestrator.Api.Tests --filter "FullyQualifiedName~D7"`.
   Follow `docs/working-rules.md`: bounded `ls`/`grep`, never `find /`.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing backend behavior intact so existing B2 and API tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
