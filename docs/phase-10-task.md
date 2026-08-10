# Phase 10 Task — Control actions UI

You are the **senior-engineer-10** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 10) for context.
Phases 1–7 are committed. Phase 10 is a Qt GUI phase that consumes the working .NET API
boundary (P7). It runs in parallel with P8 and P9.

## Objective (Phase 10)
Build a Qt control-actions UI: Schedule, Pause/Resume, and Rollback buttons wired to the API
endpoints, with confirmation dialogs and result feedback.

## API it consumes (P7, already working)
- `POST /api/schedules` body `{"id":"...","package":"...","group_id":"..."}` → `201`.
- `POST /api/schedules/{id}/pause` → `200`.
- `POST /api/schedules/{id}/resume` → `200`.
- `POST /api/schedules/{id}/rollback` → `200`.
- `GET /api/schedules/{id}/status` → `200` with `{"id":"...","status":"..."}`.

## Deliverables
1. **Control panel window** — `src/ui/control_panel.hpp` / `src/ui/control_panel.cpp`:
   - `class ControlPanelWindow : public QMainWindow`.
   - A schedule-id field and buttons: **Schedule**, **Pause**, **Resume**, **Rollback**.
   - A status label showing the current schedule status (from `GET /api/schedules/{id}/status`).
   - **Confirmation:** each destructive/control action shows a confirmation dialog before
     sending the request.
   - **Result feedback:** after each action, show the API response (success or error) in the
     status label.
2. **Entry point** — `src/ui/control_main.cpp` with `main()` that constructs and shows
   `ControlPanelWindow`.
3. **Qt CMake target** — add target **`patchorchestrator_control_ui`** to
   `src/CMakeLists.txt`:
   - `find_package(Qt6 COMPONENTS Widgets Network)`.
   - `add_executable(patchorchestrator_control_ui src/ui/control_main.cpp src/ui/control_panel.cpp)`.
   - Link `Qt6::Widgets Qt6::Network`; enable `CMAKE_AUTOMOC`.
4. **Configurability** — API base URL from env `PATCHORCH_API_URL` (default
   `http://localhost:5000`).

## Constraints / working rules
- ALWAYS run build/test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 300 bash tests/phase10/verify_ui_build.sh`).
- On this machine there is NO g++/clang on PATH. MSVC `cl` is only available inside the
  VS 2022 developer environment. Run the phase-10 runner inside it:
  ```
  cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash tests/phase10/verify_ui_build.sh"
  ```
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`tests/phase10/verify_ui_build.sh` (the scrum-master runner) builds the
`patchorchestrator_control_ui` target, confirms it links, and passes an offscreen smoke run.

## Report
Reply `DONE` on success or a concise error on failure.
