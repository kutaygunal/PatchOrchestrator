# Phase 7 Task — API <-> engine bridge

You are the **senior-engineer-7** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 7) for context.
Phases 1–6 are committed. Phase 7 depends on P5 (the .NET API, done) and P3/P4 (the Python
engine + its tests, done).

## Objective (Phase 7)
Make the .NET API drive the Python simulation engine over a defined interface, and add API
unit/integration tests. This is the bridge that lets the GUI (later phases) trigger real
simulations through the API.

## Existing pieces (do NOT break them)
- **API:** `dotnet/PatchOrchestrator.Api/` (ASP.NET Core, `net10.0`). In-memory schedule
  store with `/api/health`, `/api/schedules`, pause/resume/rollback, and status endpoints.
- **Engine:** `python/engine.py` — deterministic, seeded `engine.Rollout` / `engine.Endpoint`
  (states: pending/running/paused/failed/rolled_back/succeeded). Importable as `import engine`.

## Deliverables
1. **Python bridge script** at `python/bridge.py` that drives the engine over a JSON
   interface (see contract below).
2. **.NET bridge service** in the API project: an `IEngineBridge` interface and an
   `EngineBridge` implementation that invokes the Python bridge via subprocess and parses
   its JSON output.
3. **API integration endpoint** that uses the bridge (see contract below).
4. **API unit/integration tests** (e.g. an xUnit test project under `dotnet/`) covering the
   bridge: it calls the engine, returns parsed results, and is deterministic for a fixed
   seed.

## Bridge contract (implement exactly)

### Python side — `python/bridge.py`
- Reads a single JSON object from **stdin**:
  ```json
  {"endpoints":[{"id":"ep-1","failure_rate":0.1},{"id":"ep-2","failure_rate":0.0}],"seed":42}
  ```
- Runs `engine.Rollout(endpoints, seed).simulate()`.
- Prints a single JSON object to **stdout**:
  ```json
  {"endpoints":[{"id":"ep-1","state":"succeeded","progress":100.0}, ...],"rolled_back":false}
  ```
- Must be runnable as `python bridge.py` with `PYTHONPATH` pointing at `python/` (so
  `import engine` works). Use `python` (3.11) — NOT `python3` (a Windows Store alias
  without the needed environment).

### .NET side — `IEngineBridge`
```csharp
public interface IEngineBridge
{
    EngineResult Run(EngineRequest request);
}
```
- `EngineRequest`: `Endpoints` (list of `{Id, FailureRate}`) and `Seed` (int).
- `EngineResult`: `Endpoints` (list of `{Id, State, Progress}`) and `RolledBack` (bool).
- `EngineBridge` invokes `python bridge.py` via `System.Diagnostics.Process`, passes the
  request JSON on stdin, and deserializes the stdout JSON. Resolve the `python/` directory
  via an env var `PATCHORCH_PYTHON_DIR` (default: `../python` relative to the API project).

### API endpoint
- `POST /api/schedules/{id}/simulate` with body:
  ```json
  {"seed":42,"endpoints":[{"id":"ep-1","failure_rate":0.1},{"id":"ep-2","failure_rate":0.0}]}
  ```
- Returns `200 OK` with a JSON body containing an `endpoints` array where each element has
  `id`, `state`, and `progress` fields (the engine results).
- Returns `404 Not Found` for an unknown schedule id.
- Deterministic: the same body + same seed returns identical results.

## Constraints / working rules
- ALWAYS run build/test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 300 bash tests/phase7/verify_bridge.sh`).
- Use `python` (3.11) for any Python invocation, never `python3`.
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`tests/phase7/verify_bridge.sh` (the scrum-master harness) builds the API, runs it, and all
bridge contract checks pass (including determinism).

## Report
Reply `DONE` on success or a concise error on failure.
