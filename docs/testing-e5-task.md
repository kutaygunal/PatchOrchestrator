# Testing Agent Task — Sprint 36 (S36 / E5 Timestamp formatting)

You are **testing-e5**. Verify Sprint 36. Follow `docs/working-rules.md`.

## Sprint 36 (S36) — E5 Timestamp formatting
- **Scope:** Format ISO-8601 timestamps into a human-readable local-time display in the audit
  log panel.
- **Acceptance criteria:** Timestamps formatted to local time; readable.

## Your job
1. Read `docs/phase-e5-tests.md` (the test plan) and `docs/sprints-improvements.md` (S36).
2. Run the E5 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `e5_formatting` (timestamp formatting to readable local time)
   - `e5_display` (log panel displays formatted local-time timestamps)
   - `e5_regression` (existing A1/E4 tests still pass)
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
