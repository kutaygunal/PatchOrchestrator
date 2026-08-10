# Phase 14 Task — Polish + hardening (FINAL phase)

You are the **senior-engineer-14** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 14) for context.
Phases 1–13 are DONE and committed. Phase 14 is the **FINAL** phase: Polish + hardening
(error handling, logging, packaging/installers, final review and release). It must not regress
any prior behaviour.

## Objective (Phase 14)
Harden the application and prepare a release:
1. **Error handling + structured logging** across the three layers where gaps remain:
   - C++/Qt GUIs (`src/ui/`): consistent error handling for API failures, a small structured
     logger (timestamp + level + message) replacing ad-hoc qDebug, and graceful shutdown.
   - .NET API (`dotnet/PatchOrchestrator.Api/`): structured logging (Microsoft.Extensions
     Logging / Serilog-style) for each request, including the Python bridge call.
   - Python engine (`python/engine.py`, `python/bridge.py`): structured stdout/stderr logging
     and clear error surfaces from `bridge.py`.
2. **Packaging / installer**:
   - Add a CPack installer target for the Qt applications (root `CMakeLists.txt`):
     `include(CPack)` with a package name/version, and use `windeployqt` to bundle the Qt
     runtime DLLs into the install tree before packaging.
   - Document a release procedure (e.g. `docs/release-notes.md` or a `RELEASE` section in the
     README).
3. **Final review + release**:
   - Confirm the full test suite (P4 pytest, P6 CTest, P11 E2E) still passes.
   - Confirm the README is accurate.
   - Tag a release (e.g. `v0.1.0`).

## Required files
- Root `CMakeLists.txt`: add `include(CPack)` and CPack variables
  (`CPACK_PACKAGE_NAME`, `CPACK_PACKAGE_VERSION`, `CPACK_GENERATOR`, installer type).
- A CPack packaging step that invokes `windeployqt` on the built Qt exes (mirroring the
  P8/P9/P10 runners) before `cpack`.
- Any structured-logging and error-handling changes in `src/ui/`, `dotnet/`, `python/`.
- Release notes + a git release tag.

## Constraints / working rules
- Do NOT regress P1–P13 behaviour. Do NOT commit/push (devops agent's job). No `find /`.
- MSVC `cl` only in the VS 2022 dev env; Qt DLLs via `windeployqt`.
- HARD timeouts; run one test at a time. Re-run the full suites after any change.

## Definition of done
`tests/phase14/verify_hardening.sh` (the scrum-master authored verification harness) passes.

## Report
Reply `DONE` on success or a concise error on failure.
