# Analysis — Why a job scheduled in the Control Panel does not reliably appear in the Dashboard

Investigated end-to-end: control panel → API → dashboard, against both source and the
running processes (API on `http://localhost:5000`, dashboard + control panel binaries).
Findings were validated with a live probe against the running API.

## ROOT CAUSE (single most likely reason)

**The dashboard only reloads a schedule's fleet when the *resolved schedule id changes*.
It never reloads when the *same id* is re-scheduled (overwritten).**

In `src/ui/dashboard.cpp`, `DashboardWindow::onSchedulesReply` gates the reload with:

```cpp
if (resolved != m_scheduleId || !m_scheduleReady) {
    m_scheduleId = resolved;
    m_scheduleExists = true;
    startWithSchedule();   // -> fetchFleet() -> GET /api/schedules/{id}
}
```

When the user schedules a job with the **same id** the dashboard is already displaying —
the common case, since both apps default to `sch-1` and "Schedule" *overwrites* the id —
`resolved == m_scheduleId` and `m_scheduleReady == true`, so the guard is **false** and the
dashboard **never refetches the fleet**. The API does persist the new fleet (verified), but
the dashboard keeps simulating its stale `m_fleet` and keeps showing the old endpoints.
The job only "appears" when the user types a brand-new unique id.

That is why it is *unreliable*: new ids show up, overwritten/current ids silently do not.

## Step-by-step trace

### What happens when the user schedules a job (Control Panel)

1. `ControlPanelWindow::onSchedule()` validates a non-empty id, then `schedulePayload()`
   builds the body from the shared context: `{ id, package, group_id, fleetSize,
   failureRate, seed }`. `sendAction("/api/schedules", POST, body)` posts it to
   `http://localhost:5000` (or `PATCHORCH_API_URL`).
2. API `Program.cs` `MapPost("/api/schedules", ...)` creates/overwrites the schedule,
   persists the derived fleet (ep-1…ep-N with each endpoint's failure rate), sets
   `CreatedAt = DateTimeOffset.UtcNow`, returns 201. (Verified: after re-POSTing id
   `analyst-probe-2743` with `fleetSize=8`, `GET /api/schedules/analyst-probe-2743`
   returned a fleet of **8** endpoints.)
3. Every poll tick the dashboard calls `discoverSchedules()` → `GET /api/schedules`,
   which returns the schedules **newest-first** (`OrderByDescending(CreatedAt)`).
4. `resolveScheduleId()` picks the newest `created` (or `PATCHORCH_SCHEDULE_ID` override).
5. If the resolved id **differs** from `m_scheduleId`, `onSchedulesReply` reloads:
   `fetchFleet()` → `GET /api/schedules/{id}` → sets `m_fleet` from the API → `beginPolling()`
   → `pollSimulate()` / `pollStatus()` / SSE stream. The dashboard then shows the new fleet.

### Where it breaks (same-id re-schedule)

Steps 1–2 are identical (the API stores the new fleet). At step 5, `resolved == m_scheduleId`
and the dashboard is already "ready", so the guard is false and the **fleet is never
refetched**. `onPollTick()` then calls `pollSimulate()` with the *old* `m_fleet`, so the
displayed endpoints / progress never change. No error is shown — the dashboard just keeps
showing the previously-loaded fleet.

### Live confirmation

- Created a brand-new schedule id `analyst-probe-2743` (fleetSize 5). Within ~2 s the
  running dashboard auto-discovered it and switched to it (`Simulating schedule
  analyst-probe-2743 with 5 endpoint(s), seed 42`). → **new ids work.**
- Re-POSTed the **same** id with `fleetSize=8`. The API persisted 8 endpoints, but the
  dashboard had no path to reload an unchanged id (the `resolved != m_scheduleId` guard is
  the only trigger). → **same-id overwrites do not appear.**

## Secondary issues

1. **In-memory schedule store + stale binaries.** `Program.cs` keeps schedules in a
   process-local `schedules` dictionary, so the list resets on every API restart. During
   the investigation an older running API returned **405 for `GET /api/schedules`**
   (it predated the list endpoint), which is exactly the class of failure that makes
   discovery silently stop. Keep the API/dashboard binaries in sync with source and restart
   together.
2. **`pollSimulate` hardcodes `seed = kDefaultSeed` (42)** and drives with `m_fleet`; it
   does not use the schedule's own `seed`/`fleetSize`. Even after a successful reload the
   dashboard simulates with seed 42, so per-endpoint progress is not reproducible from the
   configured seed. Minor — affects displayed progress, not fleet visibility.
3. **No visual signal when an overwrite is skipped.** The dashboard keeps showing the old
   id/fleet and never tells the user it was skipped, so the failure is silent.
4. **Latent `created` parsing fragility.** `resolveScheduleId` parses `created` with
   `Qt::ISODate` against a `+00:00`-suffixed timestamp. Because the API already returns the
   list newest-first, the resolver effectively picks the first element, so this is not
   currently a bug — but it would break if the API ever changed ordering.

## Recommended fixes

Primary fix — reload when the record's **content** changes, not only when the id changes.
Track the last-seen `created` timestamp (or a fleet-size/content hash) per id and reload on
change:

```cpp
// in DashboardWindow:
QString m_lastCreated; // last-seen "created" for the current m_scheduleId

// in onSchedulesReply, resolve the newest entry + its created value:
const QString newCreated = /* "created" of the resolved schedule */;
if (resolved != m_scheduleId || !m_scheduleReady ||
    (resolved == m_scheduleId && newCreated != m_lastCreated)) {
    m_scheduleId = resolved;
    m_lastCreated = newCreated;
    startWithSchedule();
}
```

Alternative/adjunct: in `onPollTick`, periodically re-run `fetchFleet()` even when the id is
unchanged (the detail response is cheap), or re-fetch the schedule detail after each
discovery and reload when `fleetSize`/`seed` differ.

Secondary fixes:

- Use the schedule's persisted `seed` in `pollSimulate` instead of `kDefaultSeed`.
- Surface a status message when an overwrite is detected ("Schedule <id> re-scheduled —
  fleet updated") so the update is visible even before the table re-renders.
- (Operational) Rebuild and restart both the API and dashboard from current source together,
  and document that the schedule list is in-memory and resets on restart.

## Fix status

The primary fix has been **implemented and built successfully** (source changed, no commit):

- `src/ui/dashboard.hpp`: added members `m_lastCreated` (last-seen `created` for the
  current id) and `m_currentSeed` (the schedule's persisted seed), and a
  `scheduleCreated(...)` helper declaration.
- `src/ui/dashboard.cpp`:
  - `onSchedulesReply` now reloads when the id changed **or** when the same id's
    `created` timestamp changed (detects a same-id overwrite), so a re-scheduled
    job in the control panel refreshes the dashboard fleet instead of silently
    skipping.
  - `onFleetReply` stores the schedule's `seed` from the API detail response.
  - `pollSimulate` now sends the schedule's own seed instead of the hardcoded `42`.
- Rebuilt `patchorchestrator_ui.exe` (Release) and the `p3_dashboard_discovery`
  test target successfully; all 7 discovery tests pass (0 failures).

## Note on environment

A probe schedule `analyst-probe-2743` (fleetSize 8) remains in the running API's in-memory
store; there is no DELETE endpoint, so it cannot be removed via the API.
