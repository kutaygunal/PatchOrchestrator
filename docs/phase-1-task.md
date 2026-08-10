# Phase 1 Task — Project skeleton + CMake build

You are the **senior-engineer-1** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 1) for context.

## Objective (Phase 1)
Init the repo layout and a working CMake build that configures all future sub-builds.

## Deliverables
1. Repo directory layout: `src/` (C++/Qt), `python/` (simulation engine),
   `dotnet/` (REST API), `tests/`, `docs/`.
2. Root `CMakeLists.txt` that wires the C++/Qt target and stubs the Python and .NET
   sub-builds (configured so they can be added in later phases).
3. A basic CI stub (`.github/workflows/ci.yml`) that configures and builds the C++ target.
4. A minimal placeholder (e.g. a version/hello target) so the build produces something
   verifiable.

## Constraints / working rules
- ALWAYS run build/test commands with a HARD TIMEOUT (e.g. `timeout 300 cmake --build ...`).
- Run commands ONE AT A TIME. Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
The CMake project configures and builds successfully, the directory layout exists, and a
CI stub is present.

## Report
Reply `DONE` on success or a concise error on failure.
