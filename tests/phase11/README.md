# Phase 11 — End-to-end integration tests

Exercises the full **GUI -> API -> engine** flow against a live .NET REST API with the
Python simulation engine backend.

## Components under test
| Layer | Where | Role |
|-------|-------|------|
| Python engine | `python/engine.py`, `python/bridge.py` | Deterministic, seeded endpoint patch simulation |
| .NET REST API | `dotnet/PatchOrchestrator.Api/` | HTTP boundary + `EngineBridge` subprocess to the engine |
| Qt GUIs | `src/ui/` (P8 dashboard, P9 schedule editor, P10 control panel) | Best-effort offscreen launch check |

## Start the API + engine for the test
The `.NET` `EngineBridge` invokes `python bridge.py` via a subprocess. Point it at the Python
directory (independent of CWD), then start the API bound to the test base URL:

```bash
export PATCHORCH_PYTHON_DIR="<project-root>/python"
ASPNETCORE_URLS=http://localhost:5000 dotnet run \
  --project "<project-root>/dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj" \
  -c Release --no-build &
```

`verify_integration.sh` does exactly this automatically and cleans the API up on exit.

## Run the suite
Bash (one command, hard timeout):

```bash
timeout 300 bash tests/phase11/verify_integration.sh
```

Windows (optional, inside the VS 2022 dev env for the MSVC Qt check):

```
cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash tests/phase11/verify_integration.sh"
```

## What it verifies
1. API builds and starts, Python engine backend resolves (`PATCHORCH_PYTHON_DIR`).
2. Readiness: `GET /api/health` → 200.
3. Full HTTP flow with HTTP-code + payload-field assertions:
   - `POST /api/schedules` → 201, echoes `id`.
   - `POST /api/schedules/{id}/simulate` → 200 with `endpoints[]` (id/state/progress).
   - `POST /api/schedules/{id}/pause|resume|rollback` → 200 with the requested status.
   - `GET /api/schedules/{id}/status` → 200 with a `status` field.
   - Unknown schedule id → 404 (status, pause, simulate).
4. Best-effort offscreen launch of the built Qt executables against the running API
   (DLLs deployed with `windeployqt`). If the Qt exes or Qt bin are unavailable, the
   limitation is reported and does not fail the suite.

## Notes / constraints
- Run ONE test at a time with a HARD timeout. Never run `find /`.
- MSVC `cl` only exists inside the VS 2022 dev environment; Qt DLLs via `windeployqt`.
- This suite implements nothing — it only verifies existing P1–P10 behaviour.
