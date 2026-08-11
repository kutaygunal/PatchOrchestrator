# Testing Agent Task — Sprint 37 (S37 / E6 Log auto-refresh)

You are **testing-e6**. Verify Sprint 37. Follow `docs/working-rules.md`.

## Sprint 37 (S37) — E6 Log auto-refresh
- **Scope:** Make the audit log panel refresh in real time as actions occur (via the B5 stream
  or poll).
- **Acceptance criteria:** Panel auto-refreshes on new actions.

## Your job
1. Read `docs/phase-e6-tests.md` (the test plan) and `docs/sprints-improvements.md` (S37).
2. Run the E6 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `e6_refresh` (panel refreshes and shows a new action on an event)
   - `e6_multiple` (a sequence of events produces a corresponding sequence of updates)
   - `e6_regression` (existing E4/B5/A1 tests still pass)
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
