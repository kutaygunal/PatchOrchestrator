# Devops Agent Task — Sprint 30 (S30 / D6 Config validation)

You are **devops-d6**. Commit and push Sprint 30. Follow `docs/working-rules.md`.

## Sprint 30 (S30) — D6 Config validation
- **Scope:** Validate fleet size (≥1), failure rate (0–1), and seed (int) before starting a
  rollout; block invalid config and show inline errors.
- **Status:** Engineer implemented; testing-d6 reported PASS.

## Your job
1. Verify the working tree contains the D6 changes:
   - `src/ui/config_validator.hpp/.cpp` (new config validator)
   - `src/ui/control_panel.cpp/.hpp` (wired validation into panel)
   - `src/CMakeLists.txt` (added the new source files)
   - `tests/phase6/d6_rules_tests.cpp`, `d6_blocked_tests.cpp`, `d6_errors_tests.cpp`,
     `d6_regression_tests.cpp`
   - `tests/phase6/CMakeLists.txt` (test registration)
   - `docs/phase-d6-tests.md`, `docs/engineer-d6-task.md`, `docs/testing-d6-task.md`
2. Stage and commit the D6 implementation + tests with a conventional message, e.g.
   `feat(ui): config validation (D6)`.
3. Update `docs/sprints-improvements.md` S30 Status to "DONE" and commit as
   `docs: mark S30 (D6) as DONE`.
4. Push to origin if a remote exists; otherwise note that the repo has no remote and the
   commit is local-only.

## Constraints
- Use bounded `ls`/`grep`, never `find /`.
- Do NOT modify source code or tests.
- Run git commands with a hard timeout.

## Reply
Reply with a **single sentence**: "Pushed <commit-hash> — S30 (D6) committed and pushed." or
"Committed <hash> locally; no remote configured."
