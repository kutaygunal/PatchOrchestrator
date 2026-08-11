# Senior Engineer Task — Sprint 22 (S22 / C5 Rollout-stage grouping)

You are **senior-engineer-c5**. Implement Sprint 22. Follow `docs/working-rules.md`.

## Sprint 22 (S22) — C5 Rollout-stage grouping
- **Scope:** Group endpoints by rollout stage (wave/group) with stage headers and per-stage
  progress.
- **Acceptance criteria:** Endpoints grouped by stage; headers + per-stage progress shown.
- **Dependencies:** P9 (schedule editor, done).

## Your job
1. Read `docs/phase-c5-tests.md` (test plan) and `docs/sprints-improvements.md` (S22).
2. Read the dashboard (`src/ui/dashboard.cpp` / `.hpp`) and the C2/C3/C4 code.
3. Group endpoints in the dashboard by rollout stage (wave/group) with stage headers and
   per-stage progress.
4. Write the C5 tests named in the test plan (`c5_grouping`, `c5_headers`, `c5_progress`,
   `c5_regression`).
5. Build with a HARD TIMEOUT and run the tests ONE AT A TIME. Use the VS 2022 dev
   environment for MSVC if needed.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do NOT commit or push.
- Run build/test commands with a hard timeout.
- Keep the existing dashboard behavior and C2/C3/C4 code intact so P8/C1/C2/C3/C4 tests keep
  working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
