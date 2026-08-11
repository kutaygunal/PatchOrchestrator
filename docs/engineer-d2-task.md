# Senior Engineer Task — Sprint 26 (S26 / D2 Failure rate config)

You are **senior-engineer-d2**. Implement Sprint 26. Follow `docs/working-rules.md`.

## Sprint 26 (S26) — D2 Failure rate config
- **Scope:** UI control (slider/spin box) to set per-endpoint failure rate (0.0–1.0).
- **Acceptance criteria:** Control sets failure rate; value stored in shared state.
- **Dependencies:** A3 (shared app state), B2 (live engine session, done).

## Your job
1. Read `docs/phase-d2-tests.md` (test plan) and `docs/sprints-improvements.md` (S26).
2. Read the demo app context (`src/ui/demo_app_context.cpp` / `.hpp`) and the demo hub.
3. Add a control to set the failure rate (0.0–1.0), storing the value in the shared
   DemoAppContext.
4. Write the D2 tests named in the test plan (`d2_range`, `d2_control_context`,
   `d2_regression`).
5. Build with a HARD TIMEOUT and run the tests ONE AT A TIME. Use the VS 2022 dev
   environment for MSVC if needed.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do NOT commit or push.
- Run build/test commands with a hard timeout.
- Keep the existing behavior intact so existing tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
