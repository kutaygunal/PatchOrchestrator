# Senior Engineer Task — Sprint 25 (S25 / D1 Fleet size config)

You are **senior-engineer-d1**. Implement Sprint 25. Follow `docs/working-rules.md`.

## Sprint 25 (S25) — D1 Fleet size config
- **Scope:** UI control (spin box) to set number of endpoints in fleet before simulation.
- **Acceptance criteria:** Spin box sets fleet size; value stored in shared state.
- **Dependencies:** A3 (shared app state), B2 (live engine session, done).

## Your job
1. Read `docs/phase-d1-tests.md` (test plan) and `docs/sprints-improvements.md` (S25).
2. Read the demo app context (`src/ui/demo_app_context.cpp` / `.hpp`) and the demo hub.
3. Add a spin-box control to set the fleet size, storing the value in the shared
   DemoAppContext.
4. Write the D1 tests named in the test plan (`d1_spinbox`, `d1_control_context`,
   `d1_regression`).
5. Build with a HARD TIMEOUT and run the tests ONE AT A TIME. Use the VS 2022 dev
   environment for MSVC if needed.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do NOT commit or push.
- Run build/test commands with a hard timeout.
- Keep the existing behavior intact so existing tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
