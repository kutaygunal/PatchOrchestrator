# Devops Agent Task — Sprint 37 (S37 / E6 Log auto-refresh)

You are **devops-e6**. Commit and push Sprint 37. Follow `docs/working-rules.md`.

## Sprint 37 (S37) — E6 Log auto-refresh
- **Scope:** Make the audit log panel refresh in real time as actions occur (via the B5 stream
  or poll).
- **Status:** Engineer implemented; testing-e6 reported PASS. The orchestrator also fixed an
  A2 layout test regression caused by the E4 "Audit Log" tab (hub is now 5 tabs).

## Your job
1. Verify the working tree contains the E6 changes:
   - `src/ui/audit_log_panel.cpp/.hpp` (added auto-refresh: QTimer poll + context wiring,
     refreshNow/setRefreshInterval/stopRefresh controls)
   - `src/ui/demo_main_window.cpp/.hpp` (wired audit log refresh to the shared context)
   - `tests/phase6/e6_refresh_tests.cpp`, `e6_multiple_tests.cpp`, `e6_regression_tests.cpp`
   - `tests/phase6/CMakeLists.txt` (test registration)
   - `tests/phase6/a2_layout_tests.cpp` (**orchestrator's regression fix**: hub tab count is
     now 5, added Audit Log tab assertions) — include this file too.
   - `docs/phase-e6-tests.md`, `docs/engineer-e6-task.md`, `docs/testing-e6-task.md`
2. Stage and commit the E6 implementation + tests + the A2 regression fix with a conventional
   message, e.g. `feat(ui): audit log auto-refresh (E6)` (you may include the A2 fix in the
   same commit or as a separate `test(ui): update A2 layout tests for 5-tab hub` commit).
3. Update `docs/sprints-improvements.md` S37 Status to "DONE" and commit as
   `docs: mark S37 (E6) as DONE`.
4. Push to origin if a remote exists; otherwise note that the repo has no remote and the
   commit is local-only.

## Constraints
- Use bounded `ls`/`grep`, never `find /`.
- Do NOT modify source code or tests.
- Run git commands with a hard timeout.

## Reply
Reply with a **single sentence**: "Pushed <commit-hash> — S37 (E6) committed and pushed." or
"Committed <hash> locally; no remote configured."
