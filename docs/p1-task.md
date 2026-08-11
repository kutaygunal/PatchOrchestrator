# Phase 1 (P1) — API: store fleet on create + list schedules

**Phase tracker row (from `PLAN.md`):**

| # | Phase | Priority | Status |
|---|-------|----------|--------|
| 1 | API: store fleet on create + list schedules | High | NOT STARTED |

**Goal.** Make the API the single source of truth for a schedule's endpoint fleet so that a
later phase can point the dashboard at the API instead of hardcoding endpoints. Concretely:
persist the fleet configuration (and the derived endpoint list) when a schedule is created,
and expose a schedule-list endpoint the dashboard can call to discover newly created
schedules.

**Do NOT implement.** This file is the planner's task specification only. It is consumed by
the scrum-master (who writes the phase tests), then by `senior-engineer-p1`, then by
`testing-p1`.

---

## Why (current gaps)

- `POST /api/schedules` builds an `EngineRequest` from the configured fleet values
  (`fleetSize`, `failureRate`, `seed`) via `BuildRequestFromConfig`, but the resulting fleet
  (endpoint ids + failure rates) is **not persisted** on the stored `Schedule`. The
  `Schedule` model only holds `Id`, `Package`, `GroupId`, `Status`, and `Actions`.
- There is **no** `GET /api/schedules` endpoint, so nothing can discover which schedules
  exist or which one was most recently created.
- The dashboard therefore cannot learn the fleet from the API.

## What to deliver

### 1. Persist fleet on create

Extend the schedule data stored by `POST /api/schedules` so that a schedule records its
**fleet configuration** and its **derived endpoint list**.

- Persist the supplied config: `fleetSize`, `failureRate`, `seed`.
- Persist the derived fleet endpoints (id per endpoint, e.g. `ep-1 … ep-N`, with its failure
  rate). When no config is supplied, persist the default fleet
  (`ep-1` 0.1, `ep-2` 0.0, `ep-3` 0.3, seed 42) — the same default already produced by
  `CreateDefaultRequest()`.
- Add a **creation timestamp** (`CreatedAt`, `DateTimeOffset.UtcNow`) to each schedule so the
  list endpoint can order by it. Existing schedules created before this change are
  irrelevant (in-memory store, process restarts clear it).

### 2. Add `GET /api/schedules`

A new list endpoint returning all known schedules, **newest first** (by creation time).

- Route: `GET /api/schedules`.
- Response: a JSON array of schedule summaries. Each element must include at minimum:
  - `id`
  - `status`
  - `created` (the creation timestamp)
- Order: **newest first** (descending by `CreatedAt`). This is the contract the dashboard
  depends on to auto-select the latest schedule.
- Return `200 OK` even when the store is empty (an empty array), never 404.

### 3. API contract for the fleet (optional but recommended)

The list endpoint is the minimum for P1. It is **also acceptable and encouraged** to expose
the persisted fleet on a schedule detail (`GET /api/schedules/{id}`) if that eases P3
(dashboard fleet loading), but P3's exact contract will be decided in P3's own task file.
**Do not scope-creep** P1: the hard requirement is the persisted fleet on create plus the
list endpoint. If you add a detail endpoint, keep it simple and covered by the P1 tests.

---

## Files to touch

| File | Change |
|------|--------|
| `dotnet/PatchOrchestrator.Api/Program.cs` | Extend the `Schedule` model (or a parallel store shape) to hold fleet config + derived endpoints + `CreatedAt`. Populate these in the `POST /api/schedules` handler (including the default-fleet fallback path). Add the `GET /api/schedules` endpoint (newest first). |
| `dotnet/PatchOrchestrator.Api/Program.cs` | Reuse the existing `EngineRequestFactory` / `BuildRequestFromConfig` / `CreateDefaultRequest` helpers to derive the endpoint list. **Do not change their behavior.** |
| `dotnet/PatchOrchestrator.Api.Tests/<new test file>` | P1 tests are written by the scrum-master, not the engineer. The engineer must make the API satisfy the scrum-master's tests. |

**Do not touch:** `python/engine.py`, the C++/Qt UIs, the dashboard, the control panel, or
`EngineBridge.cs` / `EngineRequestFactory.cs` behavior.

---

## Acceptance criteria

The scrum-master's P1 test suite must pass. It will assert at minimum:

1. **Stored fleet on create (configured):** create a schedule with
   `fleetSize`, `failureRate`, `seed`; the stored schedule / detail exposes exactly `fleetSize`
   endpoints (`ep-1 … ep-N`), each with the configured failure rate and the given seed.
2. **Stored fleet on create (default):** create a schedule without config; the stored fleet is
   the default `ep-1/ep-2/ep-3` (0.1/0.0/0.3, seed 42).
3. **List endpoint exists:** `GET /api/schedules` returns `200` and a JSON array.
4. **List ordering:** after creating several schedules, `GET /api/schedules` returns them
   newest-first (correct by `created` timestamp).
5. **List includes summary fields:** each list item contains `id`, `status`, and `created`.
6. **Empty list:** `GET /api/schedules` with no schedules returns `200` with an empty array.
7. **Regression:** existing behavior is preserved — `POST /api/schedules` still returns
   `201 Created`, still starts a live `EngineSession`, and `GET /api/schedules/{id}/status`
   still works.

**Engineering note:** the store is a `ConcurrentDictionary<string, Schedule>`; keep it
thread-safe. Do not introduce a database — this is intentionally an in-memory phase.

## Definition of done

- `GET /api/schedules` exists, is ordered newest-first, and carries `id`, `status`, `created`.
- Created schedules persist their fleet (config + derived endpoints), including the default
  fallback.
- All P1 tests pass and all previously passing API tests still pass (run the full API test
  project once).
- No changes to the Python engine or the C++/Qt UIs.
