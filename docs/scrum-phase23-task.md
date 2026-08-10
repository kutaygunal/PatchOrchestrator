# Scrum-master Task — Prepare Phases 2 & 3 (parallel)

You are the **scrum-master** agent for PatchOrchestrator. Read `PLAN.md` and
`docs/working-rules.md`. Phase 1 is committed. Your job is to prepare phases 2 and 3 so
two engineers can run them IN PARALLEL.

## Phase 2 — Domain model (C++ core)
Fleet/Endpoint/Group, PatchSchedule, MaintenanceWindow, RolloutStage plain C++ data types
+ factory/validation, no GUI. High priority.

## Phase 3 — Python simulation engine
Endpoint patch state machine (pending/running/paused/failed/rolled-back), progress +
failure + rollback modeling, deterministic seeded sim. High priority.

## Deliverables (do NOT implement the phases yourself)
1. `docs/phase-2-task.md` — full engineer brief for P2 (objective, deliverables, constraints,
   definition of done).
2. `tests/phase2/` — the test script(s) that will verify P2 (e.g. a build+run script that
   compiles and checks the domain model compiles and a basic validation case). Use a HARD
   TIMEOUT wrapper. These tests are run by a separate testing agent later; they must pass
   only when the implementation is correct.
3. `docs/phase-3-task.md` — full engineer brief for P3.
4. `tests/phase3/` — test script(s) that will verify P3 (e.g. `pytest` suite + a runner
   script with `timeout`).

## Constraints / working rules
- Do NOT implement the phases — only write the briefs and the TEST harnesses/scripts.
- Run commands with HARD TIMEOUTs. No full-filesystem scans (`find /`).
- Do NOT commit or push.

## Report
Reply `DONE` with the list of files created, or a concise error on failure.
