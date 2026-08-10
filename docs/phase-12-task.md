# Phase 12 Task — CI/CD pipeline

You are the **senior-engineer-12** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 12) for context.
Phases 1–11 are DONE and committed. Phase 12 upgrades the existing P1 placeholder
`.github/workflows/ci.yml` into a real CI/CD pipeline.

## Objective (Phase 12)
Replace the P1 placeholder workflow (which only configures + builds the C++/Qt placeholder and
runs it) with a full CI pipeline that:
- Builds **ALL** CMake targets on Windows.
- Runs the Python engine pytest suite (P4).
- Runs the C++ CTest/gtest suite (P6).
- Runs the P11 end-to-end integration suite.
- Uploads the built binaries as an artifact.

Full CI only executes on GitHub (windows-latest). Do not attempt to run the full pipeline
locally; `tests/phase12/verify_ci.sh` locally verifies the config is coherent.

## Existing P1 workflow
`.github/workflows/ci.yml` currently only: checks out, installs Qt 6.8.2 win64_msvc2022_64 via
`jurplel/install-qt-action@v4`, configures `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`,
builds, and runs `build/Release/patchorchestrator.exe`.

## Required CI pipeline (implement in `.github/workflows/ci.yml`)
1. **Checkout** — `actions/checkout@v4`.
2. **Install Qt** — `jurplel/install-qt-action@v4` with `version: '6.8.2'`,
   `arch: 'win64_msvc2022_64'`, `cache: true`.
3. **Setup .NET** — `actions/setup-dotnet@v4` (the REST API targets .NET; the P11 E2E suite
   needs `dotnet`). Include a version that the repo's `dotnet/PatchOrchestrator.Api` targets.
4. **Configure** — `cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release`
   (BUILD_TESTING=ON so the gtest test executable is built and registered with CTest).
5. **Build all targets** — `cmake --build build --config Release`. This builds the five
   targets: `patchorchestrator`, `poc_domain`, `patchorchestrator_ui`,
   `patchorchestrator_schedule_ui`, `patchorchestrator_control_ui`.
6. **Python engine tests (P4)** — run `bash tests/phase4/run_tests.sh`.
7. **C++ CTest (P6)** — run `ctest --test-dir build -C Release --output-on-failure`
   (the P6 runner `tests/phase6/run_ctest.sh` also does this and can be called instead).
8. **.NET API build/test (P7)** — `dotnet build` the `PatchOrchestrator.Api` project and run
   `tests/phase5/verify_api.sh` / `tests/phase7/verify_bridge.sh` (or a `dotnet test` step).
9. **End-to-end integration (P11)** — run `bash tests/phase11/verify_integration.sh`.
10. **Deploy Qt DLLs + artifact upload** — run `windeployqt` on each built UI exe, then
    `actions/upload-artifact@v4` to upload `build/Release/` (or a staging dir with the
    deployed binaries) for the release job.

## Deliverables
1. Update `.github/workflows/ci.yml` to the pipeline above.
2. Keep every command behind a hard step timeout; run suites one at a time.
3. Ensure `tests/phase12/verify_ci.sh` passes (the scrum-master authored the local dry-run
   verification harness).

## Constraints / working rules
- Do NOT commit/push (devops agent's job). No `find /`. HARD timeouts; one command at a time.
- MSVC `cl` only exists inside the VS 2022 developer environment; in CI the
  `windows-latest` runner provides MSVC on PATH automatically.

## Definition of done
`.github/workflows/ci.yml` references all five targets and all four test suites
(P4 pytest, P6 CTest, P7 API/bridge, P11 E2E), uses `install-qt-action` with Qt 6.8.2
win64_msvc2022_64, and uploads the built binaries as an artifact; and
`tests/phase12/verify_ci.sh` passes.

## Report
Reply `DONE` on success or a concise error on failure.
