# Phase 13 — README verification harness

Checks that the top-level `README.md` exists and contains the required documentation sections,
and that referenced screenshots exist in the repo.

## Run
```bash
timeout 60 bash tests/phase13/verify_readme.sh
```

Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

## What it verifies
1. `README.md` exists at the repo root.
2. It contains the required sections:
   - Overview / introduction.
   - Architecture.
   - Prerequisites.
   - C++/Qt build & run instructions (cmake + Qt).
   - .NET API build & run instructions (dotnet + `PATCHORCH_PYTHON_DIR`).
   - Python engine instructions (python + engine).
   - Test instructions (pytest + ctest).
   - A screenshot reference.
3. Referenced screenshot files exist in the repo (e.g. `docs/screenshots/*.png`); remote
   image URLs are noted and not checked locally.

## Notes
- Run ONE command at a time with a HARD timeout. No `find /`.
- This harness verifies documentation only — it does not modify application code.
