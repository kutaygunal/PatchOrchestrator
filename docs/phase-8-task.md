# Phase 8 Task — Qt dashboard UI (read-only)

You are the **senior-engineer-8** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 8) for context.
Phases 1–7 are committed. Phase 8 is a Qt GUI phase that consumes the working .NET API
boundary (P7). It runs in parallel with P9 and P10.

## Objective (Phase 8)
Build a read-only Qt dashboard that lists simulated endpoints and their patch status in a
table, polling/refreshing against the API. Add a dedicated Qt CMake target.

## API it consumes (P7, already working)
- `POST /api/schedules/{id}/simulate` body `{"seed":42,"endpoints":[{"id":"ep-1","failure_rate":0.1}]}`
  → `200` with `{"endpoints":[{"id":"ep-1","state":"...","progress":...}]}`.
- `GET /api/schedules/{id}/status` → `200` with `{"id":"...","status":"..."}`.
- `POST /api/schedules` body `{"id":"...","package":"...","group_id":"..."}` → `201`.

## Deliverables
1. **Dashboard window** — `src/ui/dashboard.hpp` / `src/ui/dashboard.cpp`:
   - `class DashboardWindow : public QMainWindow`.
   - A `QTableWidget` with columns **Endpoint ID**, **State**, **Progress**.
   - A `QTimer` that periodically polls the API and refreshes the table (polling/refresh).
   - A refresh method that calls the API and repopulates the table from the response.
2. **Entry point** — `src/ui/dashboard_main.cpp` with `main()` that constructs and shows
   `DashboardWindow`.
3. **Qt CMake target** — add target **`patchorchestrator_ui`** to `src/CMakeLists.txt`:
   - `find_package(Qt6 COMPONENTS Widgets Network)`.
   - `add_executable(patchorchestrator_ui src/ui/dashboard_main.cpp src/ui/dashboard.cpp)`.
   - Link `Qt6::Widgets Qt6::Network`; enable `CMAKE_AUTOMOC`.
4. **Configurability** — API base URL from env `PATCHORCH_API_URL` (default
   `http://localhost:5000`); schedule id from env `PATCHORCH_SCHEDULE_ID` (default `sch-1`).
   On startup, if the schedule does not exist, create it via `POST /api/schedules`, then poll
   `POST /api/schedules/{id}/simulate` with a small default endpoint set and a fixed seed.

## Constraints / working rules
- ALWAYS run build/test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 300 bash tests/phase8/verify_ui_build.sh`).
- On this machine there is NO g++/clang on PATH. MSVC `cl` is only available inside the
  VS 2022 developer environment. Run the phase-8 runner inside it:
  ```
  cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash tests/phase8/verify_ui_build.sh"
  ```
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`tests/phase8/verify_ui_build.sh` (the scrum-master runner) builds the `patchorchestrator_ui`
target, confirms it links, and passes an offscreen smoke run.

## Report
Reply `DONE` on success or a concise error on failure.
