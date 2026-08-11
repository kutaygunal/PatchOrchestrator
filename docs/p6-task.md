# Phase 6 (P6) — Docs: describe the shared schedule behavior

**Phase tracker row (from `PLAN.md`):**

| # | Phase | Priority | Status |
|---|-------|----------|--------|
| 6 | Docs | Update README run instructions to describe the shared schedule behavior | Low | IN PROGRESS |

**Goal.** Update `README.md` so an operator can understand and run the now-connected control
panel + dashboard around a **single shared source of truth**: the control panel sends the
fleet configuration when it schedules a job, the API persists it, and the dashboard
**auto-discovers** the latest schedule and its fleet from `GET /api/schedules` — with the
`PATCHORCH_SCHEDULE_ID` override preserved.

**Do NOT implement.** This is the planner's task specification for `senior-engineer-p6`, who
updates `README.md` only. No production code changes. Do not modify the Python engine, the
API, or the C++/Qt UIs.

---

## Background (why this doc update is needed)

The README currently describes a system where the dashboard must be manually pointed at a
schedule id and the control panel does not appear to communicate with the dashboard. Since
P1–P5, the system now works through a shared schedule/fleet contract:

- **Control panel** (`patchorchestrator_control_ui`) sends `fleetSize`, `failureRate`, and
  `seed` in the `POST /api/schedules` body when the operator clicks **Schedule**.
- **API** (`GET /api/schedules`) persists that fleet and returns a schedule list ordered
  **newest-first**, with each item carrying `id`, `status`, and `created`. A schedule detail
  (`GET /api/schedules/{id}`) exposes the stored fleet (endpoints + failure rates + seed).
- **Dashboard** (`patchorchestrator_ui`) calls `GET /api/schedules`, **auto-selects the most
  recently created schedule**, and loads **that** schedule's fleet from the API — no
  hardcoded endpoints. If `PATCHORCH_SCHEDULE_ID` is set, that value overrides auto-selection.

## What to deliver

Update the README (edit, do not rewrite wholesale) to:

### 1. Describe the shared schedule behavior in the overview/architecture

- Explain that scheduling a job in the control panel makes it appear in the dashboard
  **automatically**, without restarting or manually pointing the dashboard at a schedule id.
- State the shared contract: the API is the **single source of truth** for a schedule's
  endpoint fleet; both UIs read from it.

### 2. Update the "Build & run" run instructions

- **Control panel:** show how to run it, set `PATCHORCH_API_URL`, and note that the Schedule
  action now sends the configured fleet (`fleetSize`, `failureRate`, `seed`).
- **Dashboard:** show how to run it, set `PATCHORCH_API_URL`, and document the two behaviors:
  1. With no `PATCHORCH_SCHEDULE_ID`, it auto-selects the **most recently created** schedule
     from `GET /api/schedules` and renders its fleet.
  2. With `PATCHORCH_SCHEDULE_ID` set, that value overrides auto-selection (preserved
     behavior).
- **End-to-end recipe:** give a short, copy-paste sequence — start the API on
  `http://localhost:5000`, run the control panel, schedule a job, then run the dashboard and
  confirm it shows that job and its fleet. Reference `docs/p5-verify.md` for the full
  verification steps.

### 3. Keep accuracy and consistency

- Keep the existing README structure and formatting style (bash code blocks, table of
  contents, "**How the layers work together**" numbered list). Extend rather than break.
- Keep the note that the dashboard expects the API to be running.
- Do **not** remove existing sections (Python engine, testing, packaging, career relevance).
- Ensure any environment variables mentioned are exactly `PATCHORCH_API_URL` and
  `PATCHORCH_SCHEDULE_ID`, and the API URL default is `http://localhost:5000`.

### Files to touch

| File | Change |
|------|--------|
| `README.md` | Describe the shared schedule behavior and update the run instructions as above. |

**Do not touch:** `PLAN.md`, `docs/*` task/verify files, source code, the Python engine, or
the .NET API.

---

## Acceptance criteria

The README must let a reader:

1. **Understand the shared source of truth** — scheduling in the control panel flows to the
   dashboard automatically via the API.
2. **Run the API** on `http://localhost:5000` (with `PATCHORCH_PYTHON_DIR` set) using the
   documented command.
3. **Run the control panel** and know that **Schedule** sends the configured fleet
   (`fleetSize`, `failureRate`, `seed`).
4. **Run the dashboard** and know that it:
   - auto-selects the **latest** schedule from `GET /api/schedules` and renders its fleet
     from the API (no hardcoded endpoints), and
   - honors the `PATCHORCH_SCHEDULE_ID` override when set.
5. **Follow a short end-to-end recipe** to reproduce scheduling-in-the-control-panel →
   appearing-in-the-dashboard.

## Definition of done

- README clearly describes the shared schedule/fleet contract.
- README run instructions cover the API, control panel, and dashboard with the shared
  behavior and the `PATCHORCH_SCHEDULE_ID` override.
- A short end-to-end recipe is present.
- No source/engine/API changes; README remains accurate and internally consistent.
