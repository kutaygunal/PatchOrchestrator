# Testing Agent Task — Sprint 38 (S38 / E7 Log export)

You are **testing-e7**. Verify Sprint 38. Follow `docs/working-rules.md`.

## Sprint 38 (S38) — E7 Log export
- **Scope:** Add a button to export the action log to CSV/JSON for demo handoff.
- **Acceptance criteria:** Export produces valid CSV/JSON file.

## Your job
1. Read `docs/phase-e7-tests.md` (the test plan) and `docs/sprints-improvements.md` (S38).
2. Run the E7 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `e7_export` (export produces a valid file with all entries and fields)
   - `e7_valid` (CSV/JSON is valid; content matches the log)
   - `e7_regression` (existing A1/E4 tests still pass)
   The E7 engineer built into `build.p6`. Use the VS 2022 dev environment for MSVC.
   **Important:** the Qt DLLs are at `C:/Qt/6.8.2/msvc2022_64/bin` — put that on PATH or the
   test binaries fail to launch with `0xc0000135` (DLL not found). Use bounded `ls`/`grep`,
   never `find /`.
3. If any test FAILS, report the failure concisely (which test, what assertion) so the
   engineer can fix it. If all PASS, report PASS.

## Constraints
- Do NOT implement or fix anything. Do NOT commit or push.
- Run tests one at a time with hard timeouts.

## Reply
Reply with a **single sentence**: "PASS" or "FAIL: <test> <assertion>".
