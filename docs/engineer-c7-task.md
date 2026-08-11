# Senior Engineer Task — Sprint 24 (S24 / C7 Dashboard legend)

You are **senior-engineer-c7**. Implement Sprint 24. Follow `docs/working-rules.md`.

## Sprint 24 (S24) — C7 Dashboard legend
- **Scope:** Legend explaining color coding and state meanings for demo viewers.
- **Acceptance criteria:** Legend visible; explains all states/colors.
- **Dependencies:** C2 (state→color mapping, done).

## Your job
1. Read `docs/phase-c7-tests.md` (test plan) and `docs/sprints-improvements.md` (S24).
2. Read the dashboard (`src/ui/dashboard.cpp` / `.hpp`) and the C2 state→color mapping.
3. Add a legend to the dashboard explaining the color coding and state meanings for all
   states.
4. Write the C7 tests named in the test plan (`c7_legend_renders`, `c7_all_states`,
   `c7_regression`).
5. Build with a HARD TIMEOUT and run the tests ONE AT A TIME. Use the VS 2022 dev
   environment for MSVC if needed.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do NOT commit or push.
- Run build/test commands with a hard timeout.
- Keep the existing dashboard behavior and C2/C3/C4/C5/C6 code intact so P8/C1/C2/C3/C4/C5/C6
   tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
