# Plan — Make the Control Panel and Dashboard Work Together

Purpose: Give the PatchOrchestrator control panel (`patchorchestrator_control_ui`) and the
read-only dashboard (`patchorchestrator_ui`) a **shared source of truth** so that when an
operator schedules a job in the control panel, the dashboard automatically discovers and
renders that same job and its endpoint fleet — without restarting or manually pointing the
dashboard at a schedule id.

## Current problem (why this plan exists)

- The dashboard hardcodes its endpoint fleet (`ep-1`, `ep-2`, `ep-3`, seed 42) inside
  `src/ui/dashboard.cpp`. It does not ask the API what endpoints exist.
- The control panel's **Schedule** button sends only `id`, `package`, `group_id`. It never
  sends `fleetSize`, `failureRate`, or `seed`, even though those controls exist.
- The API has **no "list schedules" endpoint** (`GET /api/schedules`), so the dashboard
  cannot discover a newly created schedule.
- Result: scheduling in the control panel never appears in the dashboard. There is no shared,
  authoritative fleet definition per schedule.

## Target design

One authoritative fleet definition per schedule, stored server-side, read by both UIs:

- Control panel sends its fleet config (`fleetSize`, `failureRate`, `seed`) when creating a
  schedule.
- API stores that fleet and exposes a schedule list (`GET /api/schedules`).
- Dashboard reads the fleet from the API instead of hardcoding endpoints, and auto-selects
  the most recently created schedule (with the existing `PATCHORCH_SCHEDULE_ID` env-var
  override preserved).

## Phases (dependency-ordered)

| # | Phase | Description | Priority | Status | Assigned to | Tests | Committed |
|---|-------|-------------|----------|--------|-------------|-------|-----------|
| 1 | API: store fleet on create + list schedules | Persist fleet config/endpoints on `POST /api/schedules`; add `GET /api/schedules` returning id, status, created time (newest first) | High | IN PROGRESS | senior-engineer-p1 | P1ApiTests.cs | |
| 2 | Control panel: send fleet config | Include `fleetSize`, `failureRate`, `seed` in the Schedule POST body from the existing controls | High | NOT STARTED | | | |
| 3 | Dashboard: auto-discover fleet | Remove hardcoded endpoints; on startup/refresh call `GET /api/schedules`, pick latest (or env override), load its fleet | High | NOT STARTED | | | |
| 4 | Tests | API integration tests (list + stored fleet + ordering); control-panel payload test; dashboard discovery logic test | High | NOT STARTED | | | |
| 5 | Build + end-to-end verify | Rebuild all targets; run both UIs; confirm scheduling in the control panel appears in the dashboard | High | NOT STARTED | | | |
| 6 | Docs | Update README run instructions to describe the shared schedule behavior | Low | NOT STARTED | | | |

## Dependencies / notes

- P1 must complete first (P2 and P3 depend on the API contract).
- P2 and P3 are independent of each other after P1 and can run in parallel.
- P4 depends on P1/P2/P3. P5 depends on P1–P4. P6 depends on P5.
- Preserve the existing `PATCHORCH_SCHEDULE_ID` env-var override in the dashboard.
- Do NOT modify the Python engine; it already works deterministically.

Update the Status and Committed columns as phases complete. Commit tracker updates as
`chore(...)` commits.
