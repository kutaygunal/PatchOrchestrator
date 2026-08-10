# Phase 6 Task — C++ core unit tests

You are the **senior-engineer-6** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 6) for context.
Phases 1–3 are committed. Phase 6 depends on P2 (domain model, done) and runs in parallel
with P4 and P5.

## Objective (Phase 6)
Write C++ unit tests (GoogleTest) covering the domain model and its validation rules, and
wire them into the CMake build with CTest.

## Deliverables
1. A GoogleTest test source under `tests/phase6/` (e.g. `tests/phase6/domain_tests.cpp`)
   covering:
   - **Factories:** valid construction sets fields correctly; invalid input throws
     `std::invalid_argument` (empty id, empty package, start≥end window, no stages).
   - **Validation:** `is_valid()` returns `true` for valid objects and `false` for invalid
     ones, and never throws.
   - **Domain types:** `PatchState` enum values; `RolloutStage.order`; `PatchSchedule`
     stores its stages.
2. Wire the tests into CMake:
   - Add a test executable target that links `poc_domain` and GoogleTest.
   - Register it with `enable_testing()` / `add_test(...)` so `ctest` runs it.
   - Fetch GoogleTest via `FetchContent` (or use a system-installed gtest if present).
   - Guard the test target behind `BUILD_TESTING` (CMake's standard option) so the
     phase-1 build is unaffected when testing is off.
3. Do NOT modify `src/domain/` (the implementation is fixed).

## Constraints / working rules
- ALWAYS run build/test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 300 bash tests/phase6/run_ctest.sh`).
- On this machine there is NO g++/clang on PATH. MSVC `cl` is only available inside the
  VS 2022 developer environment. Run the phase-6 runner inside that environment:
  ```
  cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash tests/phase6/run_ctest.sh"
  ```
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`tests/phase6/run_ctest.sh` (the scrum-master runner) configures, builds, and runs the
tests via `ctest`, exiting 0 with all tests passing.

## Report
Reply `DONE` on success or a concise error on failure.
