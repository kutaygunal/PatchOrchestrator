# Phase 9 Task — Schedule definition UI

You are the **senior-engineer-9** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 9) for context.
Phases 1–7 are committed. Phase 9 is a Qt GUI phase that consumes the working .NET API
boundary (P7). It runs in parallel with P8 and P10.

## Objective (Phase 9)
Build a Qt schedule-definition UI: forms for group, maintenance-window, and rollout-stage
editors, wired to the API to create a schedule.

## API it consumes (P7, already working)
- `POST /api/schedules` body `{"id":"...","package":"...","group_id":"..."}` → `201` with
  the created schedule echoed.

## Deliverables
1. **Schedule editor window** — `src/ui/schedule_editor.hpp` / `src/ui/schedule_editor.cpp`:
   - `class ScheduleEditorWindow : public QMainWindow`.
   - Form fields for **schedule id**, **package**, **group id**.
   - A **maintenance-window** editor (start and end fields).
   - A **rollout-stage** editor (a list of stages, each with an id, order, and group ids).
   - A **Create Schedule** button that POSTs the collected data to `POST /api/schedules`
     and shows the API response (success/error) to the user.
2. **Entry point** — `src/ui/schedule_main.cpp` with `main()` that constructs and shows
   `ScheduleEditorWindow`.
3. **Qt CMake target** — add target **`patchorchestrator_schedule_ui`** to
   `src/CMakeLists.txt`:
   - `find_package(Qt6 COMPONENTS Widgets Network)`.
   - `add_executable(patchorchestrator_schedule_ui src/ui/schedule_main.cpp src/ui/schedule_editor.cpp)`.
   - Link `Qt6::Widgets Qt6::Network`; enable `CMAKE_AUTOMOC`.
4. **Configurability** — API base URL from env `PATCHORCH_API_URL` (default
   `http://localhost:5000`).

## Constraints / working rules
- ALWAYS run build/test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 300 bash tests/phase9/verify_ui_build.sh`).
- On this machine there is NO g++/clang on PATH. MSVC `cl` is only available inside the
  VS 2022 developer environment. Run the phase-9 runner inside it:
  ```
  cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash tests/phase9/verify_ui_build.sh"
  ```
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`tests/phase9/verify_ui_build.sh` (the scrum-master runner) builds the
`patchorchestrator_schedule_ui` target, confirms it links, and passes an offscreen smoke run.

## Report
Reply `DONE` on success or a concise error on failure.
