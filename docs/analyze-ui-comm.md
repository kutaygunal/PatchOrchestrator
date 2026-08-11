# Analysis Task — Why doesn't the dashboard show what the control panel schedules?

You are a diagnostic analyst. Investigate the two-app communication in the
PatchOrchestrator repo (cwd is the repo) and determine the ROOT CAUSE of why a
job scheduled in the control panel does not reliably appear in the dashboard.

## Context
- Control panel: `patchorchestrator_control_ui` (src/ui/control_panel.cpp/.hpp) schedules jobs.
- API: .NET REST API (dotnet/PatchOrchestrator.Api) — shared source of truth.
- Dashboard: `patchorchestrator_ui` (src/ui/dashboard.cpp/.hpp) is read-only and should
  auto-discover the newest schedule + its fleet.
- Both UIs point at `http://localhost:5000`.

## What to investigate
Trace the full data path end-to-end and identify any remaining gaps:
1. Control panel: what exactly does the Schedule button send? Confirm fleetSize/failureRate/seed
   are in the POST body. Look at control_panel.cpp `onSchedule`/`schedulePayload`.
2. API: confirm POST /api/schedules persists the fleet, and GET /api/schedules (newest-first)
   + GET /api/schedules/{id} return it.
3. Dashboard: confirm it discovers schedules, loads the fleet, and re-discovers when a new
   schedule appears. Review `discoverSchedules`, `onSchedulesReply`, `onFleetReply`,
   `onPollTick`, `refreshNow`, and how/when discovery runs. Note the recent fixes (Refresh
   button, auto re-discovery on poll, m_scheduleExists guard that skips re-POST).
4. Run both UIs against the API and verify with the actual running processes whether a
   schedule made via the control panel shows up in the dashboard within a few seconds.
   Check the dashboard's log output (PATCHORCH log) and the API log (api_run.log) for errors.
5. Identify any UI-level reasons the user might not SEE the update (e.g. dashboard table not
   refreshed, schedule id shown vs discovered, status messages, window focus).

## Output
Write your findings to `docs/analyze-ui-comm-findings.md` with:
- A clear ROOT CAUSE statement (the single most likely reason it appears broken to the user).
- A step-by-step trace of what happens when the user schedules a job.
- Any secondary issues found.
- Concrete recommended fix(es), if any remain.

Use bounded `ls`/`grep` (never `find /`). Run commands with hard timeouts. Do NOT commit.
Reply DONE when the findings file is written and give a 2-3 sentence root-cause summary.
