# Testing Agent Task — Sprint 27 (S27 / D3 Seed config)

You are **testing-d3**. Verify Sprint 27. Follow `docs/working-rules.md`.

## Sprint 27 (S27) — D3 Seed config
- **Scope:** UI control (spin box) to set the deterministic seed for reproducible demos.
- **Acceptance criteria:** Control sets seed; value stored in shared state.

## Your job
1. Read `docs/phase-d3-tests.md` (the test plan) and `docs/sprints-improvements.md` (S27).
2. Run the D3 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `d3_seed` (control sets integer seed; value stored in shared context)
   - `d3_control_context` (control updates context)
   - `d3_regression` (existing A3/B2/D1/D2 tests still pass)
   Use the VS 2022 dev environment for MSVC. **Important:** the Qt DLLs are at
   `C:/Qt/6.8.2/msvc2022_64/bin` — put that on PATH or the test binaries fail to launch
   with `0xc0000135` (DLL not found). Use bounded `ls`/`grep`, never `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
