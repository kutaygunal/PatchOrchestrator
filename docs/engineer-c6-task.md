# Senior Engineer Task — Sprint 23 (S23 / C6 Smooth refresh)

You are **senior-engineer-c6**. Implement Sprint 23. Follow `docs/working-rules.md`.

## Sprint 23 (S23) — C6 Smooth refresh
- **Scope:** Use `QTimer` + interpolation (or SSE-driven updates) so progress bars animate
  without flicker.
- **Acceptance criteria:** Smooth animation; no flicker; driven by B5 stream.
- **Dependencies:** C1 (animated progress bars), B5 (streaming, done).

## Your job
1. Read `docs/phase-c6-tests.md` (test plan) and `docs/sprints-improvements.md` (S23).
2. Read the dashboard (`src/ui/dashboard.cpp` / `.hpp`), the C1 animated progress bar, and
   the B5 status stream handling.
3. Ensure progress bars animate smoothly without flicker, driven by the B5 stream (and
   QTimer interpolation).
4. Write the C6 tests named in the test plan (`c6_smooth`, `c6_no_flicker`,
   `c6_stream_driven`, `c6_regression`).
5. Build with a HARD TIMEOUT and run the tests ONE AT A TIME. Use the VS 2022 dev
   environment for MSVC if needed.

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do NOT commit or push.
- Run build/test commands with a hard timeout.
- Keep the existing dashboard behavior and C1/C2/C3/C4/C5 code intact so P8/C1/C2/C3/C4/C5
   tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
