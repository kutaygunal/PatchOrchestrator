# Senior Engineer Task — Sprint 28 (S28 / D4 Scenario presets)

You are **senior-engineer-d4**. Implement Sprint 28. Follow `docs/working-rules.md`.

## Sprint 28 (S28) — D4 Scenario presets
- **Scope:** Predefined scenarios: small clean fleet, large fleet, high-failure fleet — each
  with fleet size, failure rate, and seed.
- **Acceptance criteria:** Presets defined; each loads correct config values.
- **Dependencies:** D1–D3 (fleet size, failure rate, seed controls), A3 (shared context).

## Your job
1. Read `docs/phase-d4-tests.md` (test plan) and `docs/sprints-improvements.md` (S28).
2. Add a **scenario presets model** that defines the three predefined demo scenarios. Follow
   the existing self-contained-widget style in `src/ui/` (e.g. `demo_app_context`,
   `roadmap_model`, the D1/D2/D3 controls). A clean approach:
   - New `src/ui/demo_scenario.hpp` (and `.cpp` if it has logic) defining a `DemoScenario`
     value type with fields: `QString name`, `int fleetSize`, `double failureRate`,
     `int seed`.
   - New `src/ui/scenario_presets.hpp/.cpp` (or a static factory) exposing the three
     predefined presets: **small clean fleet** (small fleet, low failure rate, a seed),
     **large fleet** (large fleet, low failure rate, a seed), **high-failure fleet**
     (moderate/large fleet, high failure rate, a seed). Pick consistent, sensible values.
   - Provide a way to look up a preset by name and to list all presets.
3. Wire the presets into the shared `DemoAppContext` (`src/ui/demo_app_context.hpp/.cpp`)
   only if needed for tests; keep existing state intact so A3/B2/D1–D3 tests keep passing.
4. Add the new source files to `src/CMakeLists.txt` (both source lists where the other UI
   controls appear).
5. Write the D4 tests named in `docs/phase-d4-tests.md` under `tests/phase6/`:
   `d4_presets_tests.cpp` (T1 — preset data correctness), `d4_load_tests.cpp` (T2 — each
   loads correct config values), and `d4_regression_tests.cpp` (T3 — A3/B2/D1–D3 still pass).
   Register the three targets in `tests/phase6/CMakeLists.txt` as `d4_presets`, `d4_load`,
   and `d4_regression`, mirroring the D3 blocks (offscreen `QT_QPA_PLATFORM=offscreen`).
6. Build with a **HARD TIMEOUT** and run the tests **ONE AT A TIME**. Use the VS 2022 dev
   environment for MSVC. **The Qt DLLs are at `C:/Qt/6.8.2/msvc2022_64/bin` — put that on
   PATH or the test binaries fail with `0xc0000135` (DLL not found).**

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing behavior intact so existing tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
