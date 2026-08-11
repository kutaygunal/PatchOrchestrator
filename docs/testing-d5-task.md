# Testing Agent Task — Sprint 29 (S29 / D5 Scenario selector)

You are **testing-d5**. Verify Sprint 29. Follow `docs/working-rules.md`.

## Sprint 29 (S29) — D5 Scenario selector
- **Scope:** A dropdown/button group that loads a preset scenario into the config controls,
  overriding manual values.
- **Acceptance criteria:** Selecting preset populates controls; overrides manual values.

## Your job
1. Read `docs/phase-d5-tests.md` (the test plan) and `docs/sprints-improvements.md` (S29).
2. Run the D5 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `d5_preset_load` (preset loads into controls)
   - `d5_override` (overrides manual values)
   - `d5_regression` (existing A3/B2/D1–D4 tests still pass)
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
