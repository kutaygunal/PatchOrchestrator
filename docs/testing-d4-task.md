# Testing Agent Task — Sprint 28 (S28 / D4 Scenario presets)

You are **testing-d4**. Verify Sprint 28. Follow `docs/working-rules.md`.

## Sprint 28 (S28) — D4 Scenario presets
- **Scope:** Predefined scenarios: small clean fleet, large fleet, high-failure fleet — each
  with fleet size, failure rate, and seed.
- **Acceptance criteria:** Presets defined; each loads correct config values.

## Your job
1. Read `docs/phase-d4-tests.md` (the test plan) and `docs/sprints-improvements.md` (S28).
2. Run the D4 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `d4_presets` (preset data correctness)
   - `d4_load` (each loads correct config values)
   - `d4_regression` (existing A3/B2/D1–D3 tests still pass)
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
