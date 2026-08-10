# Phase 12 — CI/CD configuration verification harness

Locally "dry-runs" the CI/CD config without running the GitHub pipeline. Full CI only executes
on GitHub (`windows-latest`); this harness verifies the config is coherent.

## Run
```bash
timeout 120 bash tests/phase12/verify_ci.sh
```

Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

## What it verifies
1. `.github/workflows/ci.yml` exists and is parseable YAML (PyYAML if available, else a
   structural `jobs`/`runs-on`/`steps` grep fallback).
2. The workflow references:
   - `jurplel/install-qt-action` with Qt `6.8.2` `win64_msvc2022_64`.
   - `actions/setup-dotnet` (for the .NET API and the P11 E2E suite).
   - All five CMake targets: `patchorchestrator`, `poc_domain`,
     `patchorchestrator_ui`, `patchorchestrator_schedule_ui`,
     `patchorchestrator_control_ui`.
   - `BUILD_TESTING=ON` in the configure step (so the gtest/CTest target is built).
   - `pytest` (P4), `ctest` (P6), and `phase11` (P11 E2E) steps.
   - `actions/upload-artifact` for the built binaries.
3. The underlying test runner scripts exist:
   `tests/phase4/run_tests.sh`, `tests/phase6/run_ctest.sh`,
   `tests/phase5/verify_api.sh`, `tests/phase7/verify_bridge.sh`,
   `tests/phase11/verify_integration.sh`, `tests/phase11/run_p11.bat`.
4. `src/CMakeLists.txt` declares all five targets.

## Notes
- Run ONE command at a time with a HARD timeout. No `find /`.
- This harness implements no CI — it only validates that the engineer's pipeline config is
  coherent and complete.
