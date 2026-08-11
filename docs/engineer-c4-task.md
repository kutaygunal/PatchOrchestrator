# Senior Engineer Task — Sprint 21 (S21 / C4 Fleet summary panel)

You are **senior-engineer-c4**. Implement Sprint 21. Follow `docs/working-rules.md`.

## Sprint 21 (S21) — C4 Fleet summary panel
- **Scope:** Summary widget showing counts by state (succeeded/failed/paused/running/
  pending/rolled_back) and total.
- **Acceptance criteria:** Counts correct per state; total correct; updates on change.
- **Dependencies:** C2 (state→color mapping, done).

## Your job
1. Read `docs/phase-c4-tests.md` (test plan) and `docs/sprints-improvements.md` (S21).
2. Read the dashboard (`src/ui/dashboard.cpp` / `.hpp`) and the C2 state→color mapping.
3. Add a fleet summary panel/widget that shows counts by state and total, and updates when
   the endpoint data changes.
4. Write the C4 tests named in the test plan (`c4_aggregation`, `c4_total`, `c4_updates`,
   `c4_regression`).
5. Build with a HARD TIMEOUT and run the tests ONE AT A TIME. Use the VS 2022 dev
   environment for MSVC if needed.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do NOT commit or push.
- Run build/test commands with a hard timeout.
- Keep the existing dashboard behavior and C2/C3 code intact so P8/C1/C2/C3 tests keep
  working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
