# Senior Engineer Task — Sprint 27 (S27 / D3 Seed config)

You are **senior-engineer-d3**. Implement Sprint 27. Follow `docs/working-rules.md`.

## Sprint 27 (S27) — D3 Seed config
- **Scope:** UI control (spin box) to set the deterministic seed for reproducible demos.
- **Acceptance criteria:** Control sets seed; value stored in shared state.
- **Dependencies:** A3 (shared app state), B2 (live engine session, done).

## Your job
1. Read `docs/phase-d3-tests.md` (test plan) and `docs/sprints-improvements.md` (S27).
2. Read the existing D1/D2 controls as your template: `src/ui/fleet_size_control.cpp/.hpp`
   (D1) and `src/ui/failure_rate_control.cpp/.hpp` (D2). Copy their exact pattern.
3. Add a **seed** to the shared `DemoAppContext` (`src/ui/demo_app_context.hpp/.cpp`):
   an `int seed` with a change-only `setSeed(int)` / `seedChanged(int)` signal, default
   value `0` (mirror how `fleetSize` and `failureRate` were added). Keep existing state
   intact so A3/B2/D1/D2 tests still pass.
4. Add a new `SeedControl` widget (`src/ui/seed_control.hpp/.cpp`) following the
   FleetSizeControl pattern exactly: a `QSpinBox` named `seedSpinBox`, a constructor
   `explicit SeedControl(DemoAppContext *context = nullptr, QWidget *parent = nullptr)`,
   a `setContext(DemoAppContext*)` that reads the initial seed from the context and
   keeps the spin box and context in sync (change-only setter avoids loops), a test
   accessor `QSpinBox *spinBox() const`, and a `int seed() const` accessor. Pick a
   sensible seed range (e.g. 0–99999) with a documented default.
5. Wire the `SeedControl` into `ControlPanelWindow` (`src/ui/control_panel.cpp/.hpp`):
   add a "Seed" group box in `buildUi()` next to the Fleet and Failure Rate groups, add
   a member `SeedControl *m_seed`, and a test accessor `SeedControl *seedControl() const`.
6. Add the seed control and context to the CMake source lists:
   - `src/CMakeLists.txt` (both the demo app and the control panel source lists where
     `fleet_size_control.cpp`/`failure_rate_control.cpp` appear).
7. Write the D3 tests named in `docs/phase-d3-tests.md` under `tests/phase6/`:
   `d3_control_context_tests.cpp` (T1–T3), `d3_seed_tests.cpp` (T1–T3), and
   `d3_regression_tests.cpp`. Model them on `d2_control_context_tests.cpp`,
   `d2_range_tests.cpp`, and `d2_regression_tests.cpp` respectively. Register the three
   targets in `tests/phase6/CMakeLists.txt` as `d3_control_context`, `d3_seed`, and
   `d3_regression`, mirroring the D2 blocks (offscreen `QT_QPA_PLATFORM=offscreen`).
8. Build with a **HARD TIMEOUT** and run the tests **ONE AT A TIME**. Use the VS 2022 dev
   environment for MSVC:
   `cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && ..."`

## Constraints
- Never run `find /`. Use bounded `ls`/`grep`.
- Do **NOT** commit or push (that is the devops agent's job).
- Run every build/test command with a hard timeout; run tests one at a time.
- Keep the existing behavior intact so existing tests keep working.

## Reply
Reply with a **single sentence** summarizing what you implemented. Do not paste code.
