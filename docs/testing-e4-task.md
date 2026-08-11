# Testing Agent Task — Sprint 35 (S35 / E4 Audit log panel)

You are **testing-e4**. Verify Sprint 35. Follow `docs/working-rules.md`.

## Sprint 35 (S35) — E4 Audit log panel
- **Scope:** A `QTableWidget`/`QListView` panel in the demo hub showing the live operator
  action log.
- **Acceptance criteria:** Panel displays log; updates with new entries.

## Your job
1. Read `docs/phase-e4-tests.md` (the test plan) and `docs/sprints-improvements.md` (S35).
2. Run the E4 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `e4_render` (panel renders log entries)
   - `e4_updates` (updates with new entries)
   - `e4_regression` (existing A1/E2/E3 tests still pass)
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
