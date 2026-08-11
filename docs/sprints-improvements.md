# Sprint Breakdown — Demo-Focused Functionality Improvements

Source plan: `PLAN-improvements.md` (40 functions, phases A–E). This file is the scrum-master
sprint breakdown. No implementation is performed here. Engineers are NOT assigned yet — the
orchestrator drives execution.

Status legend: `NOT STARTED` (all sprints). Effort: S / M / L.

## Execution order (dependency-ordered)

Sprints are listed in the order they should be executed. The sequence follows the plan's
dependency graph: A1–A3 (hub foundation) first, then A4–A6 (Demo Mode) and A7–A8 (Roadmap) in
parallel, then A9–A10 (assembly). B1–B2 (live session) before B3–B5 (live endpoints/streaming),
then B6–B7. C1–C2 foundational, C4–C5 build on them, C6 depends on B5. D1–D3 base controls,
D4–D5 presets, D6–D7 wiring. E1–E3 model/recording before E4–E7 UI panel.

## Sprint table

| # | Sprint | Phase/Function | Scope | Acceptance criteria | Tests | Dependencies | Effort | Status |
|---|--------|----------------|-------|---------------------|-------|--------------|--------|--------|
| 1 | S1 | A1 Main window shell | New `patchorchestrator_demo` Qt main window hosting dashboard, schedule editor, control panel as tabs/docked panels in one app | App launches; all three existing widgets render inside one window; no regressions to existing executables | Qt widget smoke test (window constructs, panels present); existing P8–P10 UI build tests still pass | P8–P10 (existing UIs) | M | DONE |
| 2 | S2 | A2 Tab/dock manager | Central `QTabWidget`/`QDockWidget` layout manager embedding the three UI widgets; persist layout state | Layout persists across restarts; tabs/docks reorderable; state saved/restored | Qt test: layout save/restore round-trip; widget embedding test | A1 | M | DONE |
| 3 | S3 | A3 Shared app state | `DemoAppContext` (QObject) holding active schedule id, API base URL, current rollout state shared across panels | All panels read/write shared state via context; single source of truth; signals propagate changes | Qt unit test: context state set/get + signal emission | A1 | M | DONE |
| 4 | S4 | A4 Demo Mode engine | `DemoModeController` running guided scripted walkthrough (step list, narration, auto-advance, pause-on-step) | Controller advances/pauses/stops steps; narration text updates; auto-advance works | Qt unit test: step sequencing, pause, auto-advance | A1, A3 | M | DONE |
| 5 | S5 | A5 Demo script definition | Declarative JSON demo script format (load scenario, schedule, simulate, pause, resume, rollback, show roadmap) | JSON schema parses; all step types recognized; invalid script rejected with clear error | Parser unit test (valid + invalid JSON); schema validation test | A4 | M | DONE |
| 6 | S6 | A6 Demo Mode UI | `DemoModeBar` widget with Start/Next/Prev/Stop controls, current-step indicator, narration text area | Controls drive the controller; step indicator updates; narration displays | Qt widget test: button actions trigger controller; indicator updates | A4, A5 | M | DONE |
| 7 | S7 | A7 Roadmap/Future tab | `RoadmapTab` widget presenting future vision (persistence, auth, real fleet, observability, multi-tenant) as styled scrollable view | Tab renders roadmap items; scrollable; styled | Qt widget test: renders items from model | A1 | M | DONE |
| 8 | S8 | A8 Roadmap content model | Data model (JSON or C++ structs) describing roadmap items (title, description, status, target phase) rendered by A7 | Model parses/loads; A7 renders all items; fields correct | Model unit test: parse + field access | A7 | S | DONE |
| 9 | S9 | A9 Demo app entry point | New `demo_main.cpp` wiring shell, tabs, demo mode, roadmap into launchable executable | Executable launches; all components wired; no crashes on startup | Launch smoke test; component wiring test | A1–A8 | M | DONE |
| 10 | S10 | A10 Demo CMake target | CMake target `patchorchestrator_demo` building the unified hub alongside existing executables | Target builds; links correctly; existing targets unaffected | CMake configure + build test; target exists in build output | A9 | S | DONE |
| 11 | S11 | B1 Persistent engine bridge | Extend `EngineBridge` to keep a single long-lived Python subprocess (instead of spawn-per-call) so pause/resume/rollback mutate live engine state | One subprocess reused across calls; state persists between calls; clean shutdown | .NET unit test: subprocess lifecycle, state persistence across calls | P7 (existing bridge) | L | DONE |
| 12 | S12 | B2 Live engine session | Server-side `EngineSession` holding one `Rollout` instance; apply pause/resume/rollback/tick in place | Session holds live rollout; operations mutate in place; deterministic | .NET unit test: session state transitions (pause/resume/rollback/tick) | B1 | M | DONE |
| 13 | S13 | B3 API live control endpoints | Wire `POST /api/schedules/{id}/pause|resume|rollback` to call live `EngineSession` (not just return status string) | Endpoints invoke session; state changes reflected; correct HTTP responses | .NET integration test: POST endpoints mutate session state | B2 | M | DONE |
| 14 | S14 | B4 API live tick endpoint | Add `POST /api/schedules/{id}/tick` (or auto-tick loop) advancing live engine deterministically | Tick advances engine deterministically; auto-tick loop optional | .NET integration test: tick advances state deterministically | B2 | M | DONE |
| 15 | S15 | B5 Real-time status streaming | Add `GET /api/schedules/{id}/status/stream` (SSE or short-poll) for live state changes | Stream delivers state changes; client receives updates | .NET integration test: stream emits on state change | B3, B4 | L | DONE |
| 16 | S16 | B6 Dashboard live reaction | Update dashboard to re-render immediately on pause/resume/rollback events (not only poll timer) | Dashboard re-renders on events; no reliance on poll timer | Qt integration test: event triggers re-render | B5, C5 | M | DONE |
| 17 | S17 | B7 Control action feedback | Control panel shows live confirmation engine actually paused/resumed/rolled back (state diff before/after) | Panel shows before/after state diff; confirmation visible | Qt integration test: state diff displayed | B3, B6 | M | DONE |
| 18 | S18 | C1 Animated progress bars | Replace static progress text with animated `QProgressBar` per endpoint smoothly animating to target | Progress bars animate to target; per-endpoint | Qt widget test: animation reaches target value | P8 (dashboard) | M | DONE |
| 19 | S19 | C2 Color-coded states | Map each patch state to color (green=succeeded, red=failed, amber=paused, blue=running, grey=pending, purple=rolled_back) applied to rows/badges | Each state maps to correct color; applied consistently | Qt unit test: state→color mapping | P8 | S | DONE |
| 20 | S20 | C3 State badge renderer | Reusable `StateBadge` widget/icon rendering color-coded state with legend | Badge renders correct color/icon; reusable; legend present | Qt widget test: badge rendering per state | C2 | S | DONE |
| 21 | S21 | C4 Fleet summary panel | Summary widget showing counts by state (succeeded/failed/paused/running/pending/rolled_back) and total | Counts correct per state; total correct; updates on change | Qt unit test: count aggregation | C2 | M | DONE |
| 22 | S22 | C5 Rollout-stage grouping | Group endpoints by rollout stage (wave/group) with stage headers and per-stage progress | Endpoints grouped by stage; headers + per-stage progress shown | Qt unit test: grouping logic | P9 (schedule editor) | M | DONE |
| 23 | S23 | C6 Smooth refresh | Use `QTimer` + interpolation (or SSE-driven updates) so progress bars animate without flicker | Smooth animation; no flicker; driven by B5 stream | Qt integration test: smooth update path | C1, B5 | M | DONE |
| 24 | S24 | C7 Dashboard legend | Legend explaining color coding and state meanings for demo viewers | Legend visible; explains all states/colors | Qt widget test: legend renders | C2 | S | DONE |
| 25 | S25 | D1 Fleet size config | UI control (spin box) to set number of endpoints in fleet before simulation | Spin box sets fleet size; value stored in shared state | Qt unit test: control updates context | A3, B2 | S | DONE |
| 26 | S26 | D2 Failure rate config | UI control (slider/spin box) to set per-endpoint failure rate (0.0–1.0) | Control sets failure rate; value stored in shared state | Qt unit test: control updates context | A3, B2 | S | DONE |
| 27 | S27 | D3 Seed config | UI control to set deterministic seed for reproducible demos | Control sets seed; value stored in shared state | Qt unit test: control updates context | A3, B2 | S | DONE |
| 28 | S28 | D4 Scenario presets | Predefined scenarios: small clean fleet, large fleet, high-failure fleet — each with fleet size, failure rate, seed | Presets defined; each loads correct config values | Qt unit test: preset data correctness | D1–D3 | S | DONE |
| 29 | S29 | D5 Scenario selector | Dropdown/button group to load a preset scenario into config controls | Selecting preset populates controls; overrides manual values | Qt widget test: preset loads into controls | D4 | S | DONE |
| 30 | S30 | D6 Config validation | Validate fleet size (≥1), failure rate (0–1), seed (int) before starting rollout; show inline errors | Invalid config blocked; inline errors shown | Qt unit test: validation rules + error display | D1–D3 | M | DONE |
| 31 | S31 | D7 Config → engine wiring | Pass configured fleet size, failure rate, seed into live `EngineSession` when starting rollout | Config passed to session; rollout uses configured values | .NET integration test: config reaches engine | D1–D3, B2 | M | DONE |
| 32 | S32 | E1 Action log model | C++/server-side `ActionLogEntry` type (action, target, timestamp, result) | Type defined with all fields; serializable | .NET/C++ unit test: field access + serialization | A3 | S | DONE |
| 33 | S33 | E2 API action log endpoint | Add `GET /api/schedules/{id}/actions` returning recorded operator actions | Endpoint returns recorded actions; correct format | .NET integration test: endpoint returns log | B3, E1 | M | DONE |
| 34 | S34 | E3 Action recording | Record schedule/pause/resume/rollback actions (with timestamps) in `EngineSession`/API | Actions recorded with timestamps; complete log | .NET unit test: actions recorded on operations | B3, E1 | M | DONE |
| 35 | S35 | E4 Audit log panel | `QTableWidget`/`QListView` panel in demo hub showing live operator action log | Panel displays log; updates with new entries | Qt widget test: panel renders log entries | E2, A1 | M | DONE |
| 36 | S36 | E5 Timestamp formatting | Format ISO-8601 timestamps into human-readable local-time display in log panel | Timestamps formatted to local time; readable | Qt unit test: timestamp formatting | E4 | S | DONE |
| 37 | S37 | E6 Log auto-refresh | Audit log panel refreshes in real time as actions occur (via B5 stream or poll) | Panel auto-refreshes on new actions | Qt integration test: refresh on stream event | E4, B5 | M | DONE |
| 38 | S38 | E7 Log export | Button to export action log to CSV/JSON for demo handoff | Export produces valid CSV/JSON file | Qt unit test: export file format/content | E4 | S | DONE |

## Notes

- **Cross-cutting**: B5 (streaming) unblocks C6 and E6. A (hub) is the integration point tying
  B, C, D, E together for the demo.
- **Parallelizable lanes** after S3: S4–S6 (Demo Mode) and S7–S8 (Roadmap) can proceed in
  parallel; S11–S17 (B) and S18–S24 (C) can proceed in parallel once their foundations land.
- All Status values are `NOT STARTED`. No engineers assigned — orchestrator drives execution.
