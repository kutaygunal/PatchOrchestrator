# Devops Agent Task — Sprint 38 (S38 / E7 Log export)

You are **devops-e7**. Commit and push Sprint 38. Follow `docs/working-rules.md`.

## Sprint 38 (S38) — E7 Log export
- **Scope:** Add a button to export the action log to CSV/JSON for demo handoff.
- **Status:** Engineer implemented; testing-e7 reported PASS.

## Your job
1. Verify the working tree contains the E7 changes:
   - `src/ui/audit_log_panel.cpp/.hpp` (added export button + exportToFile CSV/JSON export)
   - `src/CMakeLists.txt` (added any new source files)
   - `tests/phase6/e7_export_tests.cpp`, `e7_valid_tests.cpp`, `e7_regression_tests.cpp`
   - `tests/phase6/CMakeLists.txt` (test registration)
   - `docs/phase-e7-tests.md`, `docs/engineer-e7-task.md`, `docs/testing-e7-task.md`
2. Stage and commit the E7 implementation + tests with a conventional message, e.g.
   `feat(ui): log export (E7)`.
3. Update `docs/sprints-improvements.md` S38 Status to "DONE" and commit as
   `docs: mark S38 (E7) as DONE`.
4. Push to origin if a remote exists; otherwise note that the repo has no remote and the
   commit is local-only.

## Constraints
- Use bounded `ls`/`grep`, never `find /`.
- Do NOT modify source code or tests.
- Run git commands with a hard timeout.

## Reply
Reply with a **single sentence**: "Pushed <commit-hash> — S38 (E7) committed and pushed." or
"Committed <hash> locally; no remote configured."
