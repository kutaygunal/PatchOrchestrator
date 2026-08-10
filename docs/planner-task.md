# Planner Task — Initial Phase Plan

You are the **planner** agent for PatchOrchestrator. Read `ORCHESTRATION.md` and
`docs/reborn-brief.md` (section 11) and `docs/working-rules.md` for context.

## Project
PatchOrchestrator: a Qt-based control plane for scheduling, pausing, and rolling back
fleet-wide software patches. Stack: C++/Qt GUI, Python backend simulation, .NET REST API,
CMake build. Built to demonstrate C++/Qt cross-platform engineering for the **NinjaOne
Senior Software Engineer, C++ | Patching** job.

## Your job
Produce the initial phased plan in the `PLAN.md` tracker at the repo root. Break the build
into ordered, dependency-aware phases. Suggested feature surface (refine freely):

- C++/Qt desktop dashboard listing simulated endpoints + patch status.
- Patch schedule definitions (groups, maintenance windows, rollout stages).
- Controls to schedule, pause/resume, and roll back a rollout.
- A Python simulation engine modeling endpoint patch progress/failures/rollback.
- A .NET REST API boundary (or documented interface) between GUI and engine.
- Quality: unit tests, CMake build, CI/CD, README with the NinjaOne relevance story.

## Constraints
- Update `PLAN.md` with columns: `#`, `Phase`, `Description`, `Priority`, `Status`,
  `Assigned to`, `Tests`, `Committed`.
- Keep phases small enough for a single engineer to implement and test each one.
- Order so each phase depends only on already-complete phases (the orchestrator may
  parallelize independent phases).
- Working rules: do NOT run full-filesystem scans; run nothing unbounded.

## Report
Reply `DONE` when `PLAN.md` reflects the full initial plan with Status `TODO` on each phase.
