# Phase 11 Task — End-to-end integration tests

You are the **senior-engineer-11** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 11) for context.
Phases 1–10 are DONE and committed. Phase 11 does NOT implement any app code — it writes an
automated integration suite that exercises the full **GUI -> API -> engine** flow.

## Objective (Phase 11)
Write an automated integration test suite (not a live human GUI) that verifies the whole flow:

    Qt GUI executables (P8/P9/P10)  ->  .NET REST API (P5/P7) at http://localhost:5000
                                                    |
                                                    v
                                       Python simulation engine (P3) via EngineBridge

The suite must drive the API over HTTP through: **schedule creation -> simulate/run ->
status -> pause -> resume -> rollback**, asserting HTTP codes and payload fields at each step,
and (best-effort) launch the built Qt executables offscreen against the running API.

## Deliverables
1. `tests/phase11/verify_integration.sh` — runnable bash runner with HARD timeouts that:
   - builds the .NET API, starts it (with the Python engine backend), waits for readiness.
   - drives the HTTP flow: create schedule -> simulate -> status -> pause -> resume -> rollback.
   - asserts HTTP codes + payload fields at each step.
   - best-effort launches the built Qt executables (offscreen) against the running API.
   - cleans up the API process on exit.
2. `tests/phase11/run_p11.bat` — Windows cmd wrapper around the bash runner.
3. `tests/phase11/README.md` — documents how to start the API + engine for the test.

## API contract to drive (P7, already working)
- `GET  /api/health` → `200` `{"status":"ok"}`.
- `POST /api/schedules` body `{"id":"...","package":"...","group_id":"..."}` → `201`, echoes id.
- `POST /api/schedules/{id}/simulate` body
  `{"seed":42,"endpoints":[{"id":"ep-1","failure_rate":0.0}]}` → `200`
  `{"endpoints":[{"id":"...","state":"...","progress":...}]}`.
- `POST /api/schedules/{id}/pause` → `200` `{"id":"...","status":"paused"}`.
- `POST /api/schedules/{id}/resume` → `200` `{"id":"...","status":"running"}`.
- `POST /api/schedules/{id}/rollback` → `200` `{"id":"...","status":"rolled-back"}`.
- `GET  /api/schedules/{id}/status` → `200` `{"id":"...","status":"..."}`.
- Unknown schedule id → `404`.

## How the API reaches the engine (do not change)
The .NET `EngineBridge` (P7) invokes `python bridge.py` via a subprocess. Point the bridge at
the Python dir so the engine resolves regardless of CWD:

    export PATCHORCH_PYTHON_DIR="$PROJECT_ROOT/python"

Then start the API bound to the test base URL:

    ASPNETCORE_URLS=http://localhost:5000 dotnet run --project \
      "$PROJECT_ROOT/dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj" \
      -c Release --no-build &

## Qt offscreen best-effort check
If the P8/P9/P10 build dirs exist and their executables are present, deploy Qt DLLs next to
each exe with `windeployqt.exe` (mirroring the P8/P9/P10 runners) and launch with
`QT_QPA_PLATFORM=offscreen` and a hard `timeout`. A launch that survives until the timeout
(killed, rc=124) or exits cleanly (rc=0) counts as "launched without crashing". If the exes
or Qt are unavailable, note the limitation clearly and do not fail the suite on that alone.

## Constraints / working rules
- Do NOT implement app phases. Do NOT commit/push. Do NOT run `find /`.
- MSVC `cl` is only in the VS 2022 dev env; Qt DLLs via `windeployqt`.
- HARD timeouts on all commands; run tests ONE at a time:
  `timeout 300 bash tests/phase11/verify_integration.sh`.
- Never spawn an unbounded background process; the runner must clean up the API on exit.

## Definition of done
`tests/phase11/verify_integration.sh` passes the full HTTP flow (all assertions green) and
reports the Qt offscreen check result (PASS or noted limitation).

## Report
Reply `DONE` on success or a concise error on failure.
