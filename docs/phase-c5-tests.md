# Phase C5 — Test Plan (Sprint 22: Rollout-stage grouping)

Engineer: `senior-engineer-c5`. Tester: `testing-c5`. This is the test plan for grouping
endpoints by rollout stage (wave/group) with stage headers and per-stage progress in the
dashboard.

## Context

Sprint 21 (C4) added the fleet summary panel. Sprint 22 groups endpoints in the dashboard by
rollout stage (wave/group), showing stage headers and per-stage progress. It depends on the
schedule editor (P9) which defines rollout stages.

## Acceptance criteria (from sprint)

1. Endpoints grouped by stage.
2. Headers + per-stage progress shown.

## Test plan

### T1 — Grouping logic

- **Type:** Qt unit test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Provide endpoints each assigned to a rollout stage (wave/group).
- **Assertions:**
  - Endpoints are grouped by their stage.
  - Each stage contains the correct endpoints.
  - Stages appear in the correct order (e.g. by stage order/sequence).
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c5_grouping` (or the equivalent Qt Test binary).

### T2 — Stage headers shown

- **Type:** Qt widget test.
- **Assertions:**
  - Each stage has a header (e.g. stage id/name).
  - The header is displayed above its group of endpoints.
  - The number of headers matches the number of stages.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c5_headers` (or the equivalent Qt Test binary).

### T3 — Per-stage progress

- **Type:** Qt widget test.
- **Assertions:**
  - Each stage shows its own progress (e.g. aggregated progress of its endpoints).
  - Per-stage progress is computed correctly from the stage's endpoints.
  - Progress updates when endpoint states change.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c5_progress` (or the equivalent Qt Test binary).

### T4 — Regression

- **Type:** Qt widget test.
- **Assertions:**
  - The dashboard still renders endpoint data correctly with grouping.
  - Existing dashboard tests (P8), C1–C4 tests, and P9 schedule-editor tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c5_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Group endpoints by rollout stage (wave/group) in the dashboard, with stage headers and
  per-stage progress.
- Use the stage definitions from the schedule editor (P9).
- Keep the existing dashboard behavior intact so P8/C1–C4 tests keep working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
