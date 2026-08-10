# PatchOrchestrator

A Qt-based **control plane** for scheduling, pausing, and rolling back **fleet-wide software
patches** — backed by a .NET REST API and a deterministic Python simulation engine.

PatchOrchestrator is a senior-level C++/Qt engineering showcase. It demonstrates real
cross-layer integration (desktop GUI → HTTP API → simulation engine), careful patching-domain
modeling, and disciplined quality/CI practices. It is built to mirror the kind of work a
**NinjaOne Senior C++ / Qt patching engineer** does every day.

![PatchOrchestrator dashboard](docs/screenshots/dashboard.png)
*Representative rendering of the PatchOrchestrator dashboard: a read-only table of simulated
endpoints with their patch state and progress, polled live from the .NET API.*

---

## Table of contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Build & run](#build--run)
  - [C++/Qt GUI](#cppqt-gui)
  - [.NET REST API](#net-rest-api)
  - [Python simulation engine](#python-simulation-engine)
- [Testing](#testing)
- [Packaging & release](#packaging--release)
- [NinjaOne career relevance](#ninjaone-career-relevance)

---

## Overview

**PatchOrchestrator** is a three-layer system that lets an operator plan, supervise, and
recover a software patch rollout across a fleet of endpoints:

- A **C++/Qt desktop GUI** (`src/ui/`) provides the control surface:
  - `patchorchestrator_ui` — a read-only **dashboard** listing endpoints and their patch
    status, polling the API.
  - `patchorchestrator_schedule_ui` — a **schedule-definition** editor for groups,
    maintenance windows, and rollout stages.
  - `patchorchestrator_control_ui` — a **control panel** with Schedule / Pause / Resume /
    Rollback actions, confirmation dialogs, and result feedback.
- A **.NET REST API** (`dotnet/PatchOrchestrator.Api/`) exposes the HTTP boundary for
  schedules, pause/resume/rollback, and status queries, and uses an `EngineBridge` to drive
  the simulation engine.
- A **Python simulation engine** (`python/engine.py` + `python/bridge.py`) models each
  endpoint's patch state machine deterministically, so the same configuration and seed always
  produce the same rollout.

Everything flows in one direction: the GUI calls the API, the API bridges to the engine, and
the engine's deterministic results come back to be rendered in the GUI.

---

## Architecture

```
                     ┌───────────────────────────────────────────────────┐
                     │              C++/Qt Desktop GUI                    │
                     │                                                     │
                     │  patchorchestrator_ui       (dashboard, read-only) │
                     │  patchorchestrator_schedule_ui  (schedule editor)  │
                     │  patchorchestrator_control_ui    (control panel)   │
                     └──────────────────────────┬────────────────────────┘
                                                │  HTTP (REST / JSON)
                                                ▼
                     ┌───────────────────────────────────────────────────┐
                     │            .NET REST API                           │
                     │  dotnet/PatchOrchestrator.Api/                     │
                     │  • /api/health                                     │
                     │  • /api/schedules            (create)              │
                     │  • /api/schedules/{id}/simulate                    │
                     │  • /api/schedules/{id}/pause|resume|rollback       │
                     │  • /api/schedules/{id}/status                      │
                     │  • EngineBridge (subprocess JSON bridge)           │
                     └──────────────────────────┬────────────────────────┘
                                                │  stdin/stdout JSON
                                                ▼
                     ┌───────────────────────────────────────────────────┐
                     │         Python simulation engine                    │
                     │  python/engine.py   (Endpoint + Rollout state machine)│
                     │  python/bridge.py   (JSON contract wrapper)        │
                     └───────────────────────────────────────────────────┘
```

**How the layers work together**

1. The **Qt GUI** renders a table of simulated endpoints and their patch state.
2. It issues HTTP requests against the **.NET REST API** (e.g. `POST /api/schedules`,
   `POST /api/schedules/{id}/simulate`, `.../pause`, `.../rollback`, `GET .../status`).
3. The API's **`EngineBridge`** spawns `python bridge.py` as a subprocess, passes a JSON
   request on stdin, and reads the JSON result from stdout. `PATCHORCH_PYTHON_DIR` points the
   bridge at the `python/` directory so `import engine` resolves.
4. The **Python engine** runs a deterministic, seeded rollout over the endpoint fleet
   (`pending → running → paused → running → failed / rolled_back / succeeded`) and returns
   per-endpoint state and progress.
5. The API deserializes the result and returns it over HTTP; the **Qt GUI** refreshes its
   table and status bar.

Because the engine is deterministic (fixed seed → fixed result), the whole system is easy to
test and reproduce — a key property for a patching control plane.

---

## Prerequisites

| Component | Requirement |
|-----------|-------------|
| C++/Qt GUI | **Qt 6.8.2** (msvc2022_64), **CMake ≥ 3.16**, a C++17 compiler (MSVC `cl` inside the VS 2022 dev environment, or g++/clang). |
| .NET API | .NET SDK (the project targets `net10.0`; .NET 8/9 SDKs also work to build/run via `dotnet`). |
| Python engine | Python 3.11 with `pytest` (note: on Windows prefer `python`, not the `python3` Store alias). |

Qt discovery is automatic: a plain `cmake -S . -B build` finds Qt under
`C:/Qt/6.8.2/msvc2022_64` (or via `QT_ROOT` / `QT_DIR`). If Qt is not found, the build falls
back to a plain C++ console target for the base executable and skips the Qt Widgets GUIs.

---

## Build & run

### C++/Qt GUI

Configure and build the whole C++/Qt tree:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

With MSVC inside the VS 2022 developer environment:

```
cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && cmake -S . -B build && cmake --build build --config Release"
```

This produces the executables under `build/src/Release/`:

- `patchorchestrator.exe` — base Qt console entry point.
- `patchorchestrator_ui.exe` — read-only dashboard.
- `patchorchestrator_schedule_ui.exe` — schedule-definition editor.
- `patchorchestrator_control_ui.exe` — control panel (Schedule/Pause/Resume/Rollback).

The dashboard GUI expects the .NET API to be running (see below). You can configure the API
base URL and schedule id via environment variables:

```bash
export PATCHORCH_API_URL="http://localhost:5000"
export PATCHORCH_SCHEDULE_ID="sch-1"
./build/src/Release/patchorchestrator_ui.exe
```

To run headless (e.g. on a build server or for smoke checks), use Qt's offscreen platform:

```bash
export QT_QPA_PLATFORM=offscreen
./build/src/Release/patchorchestrator_ui.exe
```

### .NET REST API

Build and run the API, pointing the `EngineBridge` at the `python/` directory (independent of
the current working directory):

```bash
dotnet build dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj -c Release
export PATCHORCH_PYTHON_DIR="<project-root>/python"
ASPNETCORE_URLS=http://localhost:5000 dotnet run \
  --project dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj \
  -c Release --no-build
```

When it is up, the OpenAPI/Swagger UI is served at `http://localhost:5000/swagger`, the
OpenAPI document at `http://localhost:5000/openapi/v1.json`, and a health check at
`GET http://localhost:5000/api/health`.

### Python simulation engine

The engine is a pure-Python package, importable as `import engine` with `PYTHONPATH` pointing
at `python/`:

```bash
cd python
PYTHONPATH="." python -c "import engine; r = engine.Rollout([engine.Endpoint('ep-1')], seed=42); r.simulate(); print(r.endpoints[0].state, r.endpoints[0].progress)"
```

The JSON bridge is runnable as `bridge.py` — it reads one JSON object on stdin and writes one
JSON object on stdout:

```bash
echo '{"endpoints":[{"id":"ep-1","failure_rate":0.1}],"seed":42}' | PYTHONPATH="." python python/bridge.py
```

---

## Testing

Run each suite **one at a time with a hard timeout** (the project's working rule — never run a
single long-lived or unbounded command).

| Phase | Suite | Command |
|-------|-------|---------|
| P4 | Python engine unit tests | `timeout 180 bash tests/phase4/run_tests.sh` |
| P6 | C++ core unit tests (CTest/gtest) | `timeout 300 bash tests/phase6/run_ctest.sh` (needs the VS 2022 dev env for MSVC `cl`) |
| P5 | .NET REST API contract | `timeout 300 bash tests/phase5/verify_api.sh` |
| P7 | API ↔ engine bridge | `timeout 300 bash tests/phase7/verify_bridge.sh` |
| P11 | End-to-end GUI → API → engine | `timeout 300 bash tests/phase11/verify_integration.sh` |
| P12 | CI/CD config dry-run | `timeout 120 bash tests/phase12/verify_ci.sh` |
| P13 | README verification | `timeout 60 bash tests/phase13/verify_readme.sh` |

Windows (MSVC) variants of the C++-based suites run inside the VS 2022 dev environment:

```
cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash tests/phase6/run_ctest.sh"
```

CI runs the full matrix on GitHub Actions (`.github/workflows/ci.yml`): it installs Qt
6.8.2 (win64_msvc2022_64), configures with `BUILD_TESTING=ON`, and runs pytest, ctest, and the
Phase 11 end-to-end suite, then uploads the built binaries as an artifact.

---

## Packaging & release

The root `CMakeLists.txt` includes a **CPack** target so the Qt applications can be packaged
into a redistributable installer. The packaging step invokes `windeployqt` (via
`cmake/bundle_qt.cmake`) to bundle the Qt runtime DLLs next to each installed executable
**before** `cpack` packages the tree, so the resulting archive runs on a machine without a
dev Qt install.

Build and package:

```bash
cmake -S . -B build.p14 -DCMAKE_BUILD_TYPE=Release
cmake --build build.p14 --config Release
cpack -B build.p14/package -C Release
```

The installer archive is produced under `build.p14/package/`. See `docs/release-notes.md` for
the release notes (current release: **v0.1.0**).

### Structured logging

All three layers write structured `[ISO-8601 timestamp] LEVEL message` logs for observability
and consistent error handling:

- **C++/Qt GUIs** log API request outcomes and errors via `src/ui/log.hpp`, and stop the
  dashboard's poll timer on window close for graceful shutdown.
- **.NET API** logs every request and the Python bridge call through Microsoft.Extensions
  Logging.
- **Python bridge** logs to stderr (keeping stdout a clean JSON contract) and returns clear
  `{"error": ...}` messages with non-zero exit codes on failure.

---

## NinjaOne career relevance

PatchOrchestrator was designed to demonstrate the skills a **NinjaOne Senior C++ / Qt
patching engineer** relies on:

- **C++/Qt desktop engineering.** Three real Qt Widgets applications (dashboard, schedule
  editor, control panel) built with CMake, automoc, and a modern C++17 core. This is direct,
  hand-written desktop work — not a scaffold.
- **Patching-domain modeling.** The domain model (`src/domain/`) models `Fleet`, `Endpoint`,
  `Group`, `PatchSchedule`, `MaintenanceWindow`, and `RolloutStage` as plain C++ types with
  validation — exactly the kind of careful modeling a patching product needs to reason about
  rollouts safely.
- **Cross-layer integration.** A desktop GUI talking to a .NET REST API that bridges to a
  Python simulation engine — a realistic, layered production architecture where each boundary
  is a clean, tested contract rather than a tangle.
- **Deterministic simulation.** The engine is seeded and deterministic, so a rollout can be
  reproduced and reasoned about. Predictability is essential for trustworthy patching tooling.
- **Quality and CI discipline.** Every layer has its own test suite (pytest, gtest/CTest,
  API/bridge integration, end-to-end), wired into a GitHub Actions pipeline with artifact
  upload. Hard timeouts and single-command test runners keep the suite reliable.

Taken together, these reflect senior-level software engineering in the patching domain: deep
C++/Qt desktop skills, careful domain modeling, clean cross-layer integration, deterministic
simulation, and the CI/quality practices that keep a fleet-patching product safe to operate.

---

## License

See the project repository for license and contribution details.
