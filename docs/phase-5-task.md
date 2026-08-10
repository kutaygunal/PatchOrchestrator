# Phase 5 Task — .NET REST API boundary

You are the **senior-engineer-5** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 5) for context.
Phases 1–3 are committed. Phase 5 depends on P2 (domain model, done) and runs in parallel
with P4 and P6.

## Objective (Phase 5)
Build an ASP.NET Core REST API boundary exposing schedule, pause/resume, rollback, and
status-query operations, with a documented contract (OpenAPI). This phase is the API
boundary only — it does NOT need to call the Python engine yet (that is phase 7).

## Deliverables
1. A .NET project at **`dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj`**
   (ASP.NET Core, target framework `net10.0`). The phase-5 test harness builds and runs
   this exact project path.
2. Implement the **REST contract below exactly** (the phase-5 test harness depends on it).
3. Enable OpenAPI/Swagger so the contract is documented (e.g. `AddOpenApi()` /
   `MapOpenApi()` on .NET 10, or Swashbuckle).

## REST contract (implement exactly)

Base path: `/api`. All request/response bodies are JSON. The API must listen on a port
that the test harness can reach (default `http://localhost:5000`; the harness overrides
the port via the `ASPNETCORE_URLS` environment variable).

| Method | Path | Body | Success |
|--------|------|------|---------|
| GET  | `/api/health` | — | 200, body contains `"ok"` |
| POST | `/api/schedules` | `{"id":"sch-1","package":"pkg-v2","group_id":"grp-1"}` | 201, body echoes `id` |
| POST | `/api/schedules/{id}/pause` | — | 200 |
| POST | `/api/schedules/{id}/resume` | — | 200 |
| POST | `/api/schedules/{id}/rollback` | — | 200 |
| GET  | `/api/schedules/{id}/status` | — | 200, body contains `"status"` |

Notes:
- `POST /api/schedules` returns `201 Created` and a JSON body that includes the `id`.
- The `{id}` path operations return `200 OK` for a known schedule id and `404 Not Found`
  for an unknown id.
- `GET /api/schedules/{id}/status` returns a JSON object with a `status` field (e.g.
  `{"id":"sch-1","status":"pending"}`).
- Keep the API in-memory (no database) for this phase.

## Constraints / working rules
- ALWAYS run build/test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 300 bash tests/phase5/verify_api.sh`).
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`tests/phase5/verify_api.sh` (the scrum-master harness) builds the project, runs it, and
all contract checks pass.

## Report
Reply `DONE` on success or a concise error on failure.
