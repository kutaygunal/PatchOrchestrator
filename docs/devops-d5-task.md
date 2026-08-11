# Devops Agent Task — Sprint 29 (S29 / D5 Scenario selector)

You are **devops-d5**. Commit and push Sprint 29. Follow `docs/working-rules.md`.

## Sprint 29 (S29) — D5 Scenario selector
- **Scope:** A dropdown/button group that loads a preset scenario into the config controls,
  overriding manual values.
- **Status:** Engineer implemented; testing-d5 reported PASS.

## Your job
1. Verify the working tree contains the D5 changes:
   - `src/ui/scenario_selector.hpp/.cpp` (new scenario selector widget)
   - `src/ui/control_panel.cpp/.hpp` (wired selector into panel)
   - `src/CMakeLists.txt` (added the new source files)
   - `tests/phase6/d5_preset_load_tests.cpp`, `d5_override_tests.cpp`,
     `d5_regression_tests.cpp`
   - `tests/phase6/CMakeLists.txt` (test registration)
   - `docs/phase-d5-tests.md`, `docs/engineer-d5-task.md`, `docs/testing-d5-task.md`
2. Stage and commit the D5 implementation + tests with a conventional message, e.g.
   `feat(ui): scenario selector (D5)`.
3. Update `docs/sprints-improvements.md` S29 Status to "DONE" and commit as
   `docs: mark S29 (D5) as DONE`.
4. Push to origin if a remote exists; otherwise note that the repo has no remote and the
   commit is local-only.

## Constraints
- Use bounded `ls`/`grep`, never `find /`.
- Do NOT modify source code or tests.
- Run git commands with a hard timeout.

## Reply
Reply with a **single sentence**: "Pushed <commit-hash> — S29 (D5) committed and pushed." or
"Committed <hash> locally; no remote configured."
