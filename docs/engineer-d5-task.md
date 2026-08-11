# Senior Engineer Task — Sprint 29 (S29 / D5 Scenario selector)

You are **senior-engineer-d5**. Implement Sprint 29. Follow `docs/working-rules.md`.

## Sprint 29 (S29) — D5 Scenario selector
- **Scope:** A dropdown/button group to load a preset scenario into the config controls
  (fleet size, failure rate, seed), overriding manual values.
- **Acceptance criteria:** Selecting preset populates controls; overrides manual values.
- **Dependencies:** D4 (scenario presets, done), D1–D3 (config controls), A3 (context).

## Your job
1. Read `docs/phase-d5-tests.md` (test plan) and `docs/sprints-improvements.md` (S29).
2. Read the D4 presets API in `src/ui/scenario_presets.hpp/.cpp` and `src/ui/demo_scenario.hpp`
   (provides `scenario_presets::all()` / `byName()`). Also read the D1/D2/D3 controls
   (`fleet_size_control`, `failure_rate_control`, `seed_control`) and the shared
   `DemoAppContext` (A3, which gained a seed and an `applyScenario` helper in D3/D4).
3. Add a new self-contained **ScenarioSelector** widget (`src/ui/scenario_selector.hpp/.cpp`)
   following the established widget pattern (like the other controls):
   - A `QComboBox` (objectName `scenarioComboBox`) listing all presets from
     `scenario_presets::all()` (plus a placeholder "Select scenario…" entry).
   - A constructor `explicit ScenarioSelector(DemoAppContext *context = nullptr,
     QWidget *parent = nullptr)` and a `setContext(DemoAppContext*)` binding.
   - When the user selects a preset, apply its fleet size, failure rate, and seed to the
     shared `DemoAppContext` (using `applyScenario` if present, else the individual setters),
     which propagates to the D1/D2/D3 controls via their context bindings. This overrides any
     manually set values.
   - Test accessors: `QComboBox *comboBox() const` and the count of presets.
4. Wire `ScenarioSelector` into `ControlPanelWindow` (`src/ui/control_panel.cpp/.hpp`): add a
   "Scenario" group box in `buildUi()` (e.g. above the Fleet group), a member
   `ScenarioSelector *m_scenario`, a test accessor `ScenarioSelector *scenarioSelector() const`,
   and bind it to the panel's context so selecting a preset populates the other controls.
5. Add the new source files to `src/CMakeLists.txt` (both source lists where the other UI
   controls appear).
6. Write the D5 tests named in `docs/phase-d5-tests.md` under `tests/phase6/`:
   `d5_preset_load_tests.cpp` (T1), `d5_override_tests.cpp` (T2), and
   `d5_regression_tests.cpp` (T3). Register the three targets in
   `tests/phase6/CMakeLists.txt` as `d5_preset_load`, `d5_override`, and `d5_regression`,
   mirroring the D4/D3 blocks (offscreen `QT_QPA_PLATFORM=offscreen`).
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
