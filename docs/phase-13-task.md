# Phase 13 Task — README + NinjaOne relevance story

You are the **senior-engineer-13** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 13) for context.
Phases 1–12 are DONE and committed. Phase 13 is a **Low-priority, non-blocking** documentation
phase: produce a polished README with architecture docs, run/build instructions, screenshots,
and a NinjaOne career-relevance narrative. It does NOT change application code.

## Objective (Phase 13)
Write a top-level `README.md` that presents PatchOrchestrator as a senior-level C++/Qt
engineering showcase targeting the **NinjaOne Senior C++ Patching** role.

## Deliverables — the README MUST contain these sections
1. **Overview** — what PatchOrchestrator is: a Qt-based control plane for scheduling, pausing,
   and rolling back fleet-wide software patches, backed by a .NET REST API and a Python
   simulation engine.
2. **Architecture** — a diagram + description of the three layers:
   - C++/Qt GUI (`patchorchestrator_ui`, `patchorchestrator_schedule_ui`,
     `patchorchestrator_control_ui`).
   - .NET REST API (`dotnet/PatchOrchestrator.Api/`) + `EngineBridge`.
   - Python simulation engine (`python/engine.py`, `python/bridge.py`).
   Show how GUI -> API -> engine flow together.
3. **Prerequisites** — Qt 6.8.2 (msvc2022_64), CMake ≥ 3.16, a C++17 compiler (MSVC cl inside
   the VS 2022 dev env, or g++/clang), .NET SDK, Python 3.11 + pytest.
4. **Build & run instructions** for all three layers:
   - C++/Qt GUI (CMake configure/build + how to run offscreen or with a display).
   - .NET API (`dotnet run` with `PATCHORCH_PYTHON_DIR` pointing at `python/`).
   - Python engine (importable `engine` + runnable `bridge.py`).
5. **Test instructions** — how to run the suites: P4 pytest, P6 CTest/gtest, P7 API/bridge,
   P11 end-to-end integration, P12 CI dry-run.
6. **Screenshot(s)** — at least one screenshot (e.g. `docs/screenshots/dashboard.png`) with a
   caption, placed under `docs/screenshots/` or a `screenshots/` dir.
7. **NinjaOne relevance story** — a narrative tying the work to the **NinjaOne Senior C++
   Patching** job: C++/Qt desktop engineering, patching-domain domain modeling, cross-layer
   integration, deterministic simulation, and quality/CI practices.

## Deliverables — files
1. `README.md` at the repo root with all sections above.
2. At least one screenshot image under `docs/screenshots/` (or `screenshots/`) referenced by
   the README.

## Constraints / working rules
- Do NOT modify application code. Do NOT commit/push (devops agent's job). No `find /`.
- HARD timeouts; one command at a time. This phase is non-blocking and Low priority.
- Capturing real screenshots: the Qt UI targets can be launched offscreen via
  `QT_QPA_PLATFORM=offscreen` (see the P8/P9/P10 runners); for a real screenshot use a window
  system. If a real screenshot is not feasible on this machine, a generated/representative
  image with a clear caption is acceptable.

## Definition of done
`tests/phase13/verify_readme.sh` (the scrum-master authored verification harness) passes.

## Report
Reply `DONE` on success or a concise error on failure.
