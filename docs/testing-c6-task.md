# Testing Agent Task — Sprint 23 (S23 / C6 Smooth refresh)

You are **testing-c6**. Verify Sprint 23. Follow `docs/working-rules.md`.

## Sprint 23 (S23) — C6 Smooth refresh
- **Scope:** Use `QTimer` + interpolation (or SSE-driven updates) so progress bars animate
  without flicker.
- **Acceptance criteria:** Smooth animation; no flicker; driven by B5 stream.

## Your job
1. Read `docs/phase-c6-tests.md` (the test plan) and `docs/sprints-improvements.md` (S23).
2. Run the C6 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `c6_smooth` (smooth animation)
   - `c6_no_flicker` (no flicker)
   - `c6_stream_driven` (driven by B5 stream)
   - `c6_regression` (dashboard still renders; P8/C1/C2/C3/C4/C5 tests still pass)
   Use the VS 2022 dev environment for MSVC if needed. Use bounded `ls`/`grep`, never
   `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
