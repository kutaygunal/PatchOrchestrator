# Devops Agent Task — Sprint 28 (S28 / D4 Scenario presets)

You are **devops-d4**. Commit and push Sprint 28. Follow `docs/working-rules.md`.

## Sprint 28 (S28) — D4 Scenario presets
- **Scope:** Predefined scenarios: small clean fleet, large fleet, high-failure fleet — each
  with fleet size, failure rate, and seed.
- **Status:** Engineer implemented; testing-d4 reported PASS.

## Your job
1. Verify the working tree contains the D4 changes:
   - `src/ui/demo_scenario.hpp` and `src/ui/scenario_presets.hpp/.cpp` (new scenario model
     and presets)
   - `src/CMakeLists.txt` (added the new source files)
   - `tests/phase6/d4_presets_tests.cpp`, `d4_load_tests.cpp`, `d4_regression_tests.cpp`
   - `tests/phase6/CMakeLists.txt` (test registration)
   - `docs/phase-d4-tests.md`, `docs/engineer-d4-task.md`, `docs/testing-d4-task.md`
2. Stage and commit the D4 implementation + tests with a conventional message, e.g.
   `feat(ui): scenario presets (D4)`.
3. Update `docs/sprints-improvements.md` S28 Status to "DONE" and commit as
   `docs: mark S28 (D4) as DONE`.
4. Push to origin if a remote exists; otherwise note that the repo has no remote and the
   commit is local-only.

## Constraints
- Use bounded `ls`/`grep`, never `find /`.
- Do NOT modify source code or tests.
- Run git commands with a hard timeout.

## Reply
Reply with a **single sentence**: "Pushed <commit-hash> — S28 (D4) committed and pushed." or
"Committed <hash> locally; no remote configured."
