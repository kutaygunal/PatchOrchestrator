# Phase 14 — Polish + hardening verification harness

Verifies the P14 hardening work is coherent:
- C++/Qt code compiles with warnings-as-errors (the `poc_domain` library already enables
  `/W4 /WX` / `-Werror`).
- A CPack installer config exists and, best-effort, CPack runs (with `windeployqt` bundling).
- A release tag / release notes exist.
- The full test suites (P4 pytest, P6 CTest, P11 E2E) still pass.

## Run
```bash
# For the most complete result, run inside the VS 2022 dev env:
cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && timeout 600 bash tests/phase14/verify_hardening.sh"
```

Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

## What it verifies
1. Warnings-as-errors flags are present in `src/CMakeLists.txt` for `poc_domain`.
2. `CMakeLists.txt` includes `include(CPack)` with `CPACK_PACKAGE_NAME` / `CPACK_GENERATOR`,
   and (best-effort) `cpack` runs successfully against a configured build.
3. A git release tag and/or a release-notes file exist.
4. The P4 pytest, P6 CTest, and P11 E2E suites run (each behind a hard timeout, one at a
   time). Suites whose toolchain/env is unavailable are SKIPPED with a NOTE rather than
   failing, so the harness is usable outside the full dev environment.

## Notes
- Run ONE command at a time with a HARD timeout. No `find /`.
- MSVC `cl` only exists inside the VS 2022 developer environment; Qt DLLs via `windeployqt`.
