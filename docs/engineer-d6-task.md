# Senior Engineer Task — Sprint 30 (S30 / D6 Config validation)

You are **senior-engineer-d6**. Implement Sprint 30. Follow `docs/working-rules.md`.

## Sprint 30 (S30) — D6 Config validation
- **Scope:** Validate fleet size (≥1), failure rate (0–1), and seed (int) before starting a
  rollout; block invalid config and show inline errors.
- **Acceptance criteria:** Invalid config blocked; inline errors shown.
- **Dependencies:** D1–D3 (config controls), A3 (shared context).

## Your job
1. Read `docs/phase-d6-tests.md` (test plan) and `docs/sprints-improvements.md` (S30).
2. Read the D1/D2/D3 controls (`fleet_size_control`, `failure_rate_control`, `seed_control`),
   the shared `DemoAppContext` (A3), and the `ControlPanelWindow` (`src/ui/control_panel.cpp`
   `buildUi()`).
3. Add a **config validator** following the established self-contained style in `src/ui/`:
   - New `src/ui/config_validator.hpp` (and `.cpp` if it has logic) with a
     `ConfigValidator::Result` (or similar) that reports validity plus a human-readable
     message per field:
     - fleet size < 1 → invalid ("fleet size must be ≥ 1").
     - failure rate < 0 or > 1 → invalid ("failure rate must be between 0 and 1").
     - seed is an integer → always valid (the QSpinBox only stores ints), but keep the check
       for completeness.
     - a valid config passes.
   - Provide a `validate(fleetSize, failureRate, seed)` entry point returning the result, and
     a way to get the error message for the offending field(s).
4. Wire validation into `ControlPanelWindow` so that before the Schedule/rollback actions
   start, the config is validated; if invalid, the action is **blocked** and an **inline error**
   label near the relevant control shows the message. Add a validation error label member
   (e.g. `QLabel *m_validationLabel`) and an accessor so tests can read it. The error clears
   when the value is corrected.
5. Add the new source files to `src/CMakeLists.txt` (both source lists where the other UI
   controls appear).
6. Write the D6 tests named in `docs/phase-d6-tests.md` under `tests/phase6/`:
   `d6_rules_tests.cpp` (T1), `d6_blocked_tests.cpp` (T2), `d6_errors_tests.cpp` (T3), and
   `d6_regression_tests.cpp` (T4). Register the four targets in `tests/phase6/CMakeLists.txt`
   as `d6_rules`, `d6_blocked`, `d6_errors`, and `d6_regression`, mirroring the D5 blocks
   (offscreen `QT_QPA_PLATFORM=offscreen`).
7. Build with a **HARD TIMEOUT** and run the tests **ONE AT A TIME**. Use the VS 2022 dev
   environment for MSVC. **The Qt DLLs are at `C:/Qt/6.8.2/msvc2022_64/bin` — put that on
   PATH or the test binaries fail with `0xc0000135` (DLL not found).**

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing behavior intact so existing tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
