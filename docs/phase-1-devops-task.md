# Devops Task — Phase 1 (Project skeleton + CMake build)

You are the **devops-1** agent for PatchOrchestrator. Read `docs/working-rules.md`.

## Your job
Commit the completed Phase 1 work to git.

## Deliverables
1. Stage all new files (`src/`, `python/`, `dotnet/`, `tests/`, `docs/`, `.github/`,
   `CMakeLists.txt`, `.gitignore`, `ORCHESTRATION.md`, `PLAN.md`).
2. Create an initial commit with a clear message, e.g. `feat: project skeleton + CMake build`.
3. Try to push to `origin` (if a remote is configured). If NO remote is configured,
   commit locally only and report that push is deferred.

## Constraints / working rules
- ONLY you (devops) commit and push — never engineer/testing.
- Do NOT run a full-filesystem scan (no `find /`).
- Run commands with a HARD TIMEOUT.
- Do NOT modify source files; only stage/commit existing work.

## Definition of done
A local git commit containing all Phase 1 files exists. If a remote exists, it is pushed.

## Report
Reply `DONE` with the commit hash, or a concise error on failure.
