# Testing Agent Task — Sprint 30 (S30 / D6 Config validation)

You are **testing-d6**. Verify Sprint 30. Follow `docs/working-rules.md`.

## Sprint 30 (S30) — D6 Config validation
- **Scope:** Validate fleet size (≥1), failure rate (0–1), and seed (int) before starting a
  rollout; block invalid config and show inline errors.
- **Acceptance criteria:** Invalid config blocked; inline errors shown.

## Your job
1. Read `docs/phase-d6-tests.md` (the test plan) and `docs/sprints-improvements.md` (S30).
2. Run the D6 tests named in the test plan ONE AT A TIME with a HARD TIMEOUT:
   - `d6_rules` (validation rules)
   - `d6_blocked` (invalid config blocked)
   - `d6_errors` (inline errors shown)
   - `d6_regression` (existing A3/B2/D1–D5 tests still pass)
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
