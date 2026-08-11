# Phase 5 (P5) — Build + End-to-End Verification Checklist

**Phase tracker row (from `PLAN.md`):**

| # | Phase | Priority | Status |
|---|-------|----------|--------|
| 5 | Build + end-to-end verify | High | IN PROGRESS |

**Goal.** Rebuild every target, start the API, run both UIs, schedule a job in the control
panel, and confirm the dashboard **auto-discovers and renders that same schedule and its
fleet** — with **no hardcoded endpoints** in the dashboard.

**Do NOT implement.** This file is the scrum-master's verification checklist. It is consumed
by `senior-engineer-p5` (who runs it), then by `testing-p5`, then by `devops-p5`.

---

## Preconditions

- P1–P4 are DONE and pushed (commit `3816de9`).
- The Python engine (`python/engine.py`) is **not** modified.
- Qt 6.8.2 is at `C:/Qt/6.8.2/msvc2022_64` (add `C:/Qt/6.8.2/msvc2022_64/bin` to `PATH` to
  run the built `.exe`s).
- MSVC 2022 developer environment is available at
  `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat`.
- Repo root: `C:/Users/kutay/Desktop/Projects/PatchOrchestrator`.

> **Working-rules note:** every build/test/curl command below runs with a **HARD TIMEOUT**.
> The API is started in its **own dedicated terminal/pane** (not backgrounded from the
> verify script), so no long-running process is spawned by a single shell command.

---

## Step 1 — Rebuild all targets

### 1a. C++/Qt control panel + dashboard (MSVC vcvars64)

Run from the repo root, with a hard timeout:

```bash
timeout 600 cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release --target patchorchestrator_ui patchorchestrator_control_ui -- /m"
```

**Pass criteria (1a):**
- Exit code `0`.
- Build log ends with `BUILD_DONE` (or no error lines) and **0 errors**.
- Both executables exist:
  - `build/src/Release/patchorchestrator_ui.exe`
  - `build/src/Release/patchorchestrator_control_ui.exe`

### 1b. .NET REST API

```bash
timeout 300 dotnet build dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj -c Release
```

**Pass criteria (1b):**
- Exit code `0`, **0 warnings / 0 errors**.
- Output DLL present: `dotnet/PatchOrchestrator.Api/bin/Release/net10.0/PatchOrchestrator.Api.dll`.

---

## Step 2 — Start the API on localhost:5000

Start the API in a **dedicated terminal/pane** (do not background it from the verify
script). From the repo root:

```bash
export PATCHORCH_PYTHON_DIR="C:/Users/kutay/Desktop/Projects/PatchOrchestrator/python"
ASPNETCORE_URLS=http://localhost:5000 dotnet run \
  --project dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj \
  -c Release --no-build
```

**Pass criteria (2):**
- The process stays up and logs "Now listening on: http://localhost:5000".
- Health check returns `200` with `{"status":"ok"}`:

```bash
timeout 30 curl -s -o /dev/null -w "%{http_code}\n" http://localhost:5000/api/health
# expected: 200
```

---

## Step 3 — Verify the API contract (automated, no GUI)

Run these from a second terminal while the API is up. Each uses a hard timeout.

### 3a. Empty list (P1 contract)

```bash
timeout 30 curl -s http://localhost:5000/api/schedules
# expected: []  (200, empty JSON array)
```

### 3b. Create a schedule with a distinct fleet config

```bash
timeout 30 curl -s -X POST http://localhost:5000/api/schedules \
  -H "Content-Type: application/json" \
  -d '{"id":"e2e-1","package":"pkg-v2","group_id":"grp-1","fleetSize":5,"failureRate":0.4,"seed":123}'
# expected: 201 Created
```

### 3c. List shows it newest-first

```bash
timeout 30 curl -s http://localhost:5000/api/schedules
# expected: an array whose first element has "id":"e2e-1", "status":"running", and a "created" timestamp
```

### 3d. Detail exposes the stored fleet (no hardcoded endpoints)

```bash
timeout 30 curl -s http://localhost:5000/api/schedules/e2e-1
# expected: "fleet" = [{"id":"ep-1","failureRate":0.4}, ... {"id":"ep-5","failureRate":0.4}], "seed":123
```

**Pass criteria (3):**
- 3a returns `[]`.
- 3b returns `201`.
- 3c first element is `e2e-1`.
- 3d fleet has exactly **5** endpoints `ep-1 … ep-5`, each `failureRate` `0.4`, `seed` `123`.

---

## Step 4 — Run the control panel and schedule a job (GUI)

From the repo root, in a terminal with the Qt bin on `PATH`:

```bash
export PATH="C:/Qt/6.8.2/msvc2022_64/bin:$PATH"
export PATCHORCH_API_URL="http://localhost:5000"
./build/src/Release/patchorchestrator_control_ui.exe
```

**Manual steps:**
1. Set **Schedule ID** to `e2e-1`.
2. Set **Fleet** to `5`, **Failure Rate** to `0.4`, **Seed** to `123`.
3. Click **Schedule** and confirm the dialog.
4. Confirm the status label shows success (no error).

**Pass criteria (4):**
- The Schedule POST succeeds (no error in the status label).
- The API now stores `e2e-1` with the configured fleet (re-check with Step 3d).

---

## Step 5 — Run the dashboard and confirm auto-discovery (GUI)

From the repo root, in a **separate** terminal with the Qt bin on `PATH`. **Do NOT set
`PATCHORCH_SCHEDULE_ID`** — the dashboard must auto-select the newest schedule:

```bash
export PATH="C:/Qt/6.8.2/msvc2022_64/bin:$PATH"
export PATCHORCH_API_URL="http://localhost:5000"
./build/src/Release/patchorchestrator_ui.exe
```

**Manual steps:**
1. Let the dashboard start and poll.
2. Confirm the dashboard renders the **same schedule** (`e2e-1`) and exactly **5** endpoints
   `ep-1 … ep-5` — **not** the old hardcoded `ep-1/ep-2/ep-3` default.

**Pass criteria (5):**
- The dashboard shows schedule `e2e-1`.
- The table has **5** rows: `ep-1`, `ep-2`, `ep-3`, `ep-4`, `ep-5`.
- No hardcoded endpoint set is used (the fleet comes from the API detail, not from
  `dashboard.cpp` constants).

---

## Step 6 — Env-override regression (optional but recommended)

Repeat Step 5 but set the override to a different schedule id and confirm it wins:

```bash
export PATCHORCH_SCHEDULE_ID="sch-1"
./build/src/Release/patchorchestrator_ui.exe
```

**Pass criteria (6):**
- The dashboard selects `sch-1` (the override), not the newest schedule.

---

## Cleanup

- Close both UI windows.
- Stop the API process (Ctrl+C in its pane).
- Optionally remove the build artifacts: `rm -rf build` (bounded, repo-local only).

---

## Definition of done (P5)

- All targets rebuild cleanly (C++/Qt control panel + dashboard, and the .NET API).
- API runs on `http://localhost:5000`; `/api/health` returns `200`.
- A job scheduled in the control panel (`e2e-1`, fleet 5 / 0.4 / seed 123) is stored by the
  API and appears in `GET /api/schedules` newest-first.
- The dashboard **auto-discovers** that schedule and renders its **5** endpoints from the
  API — with **no hardcoded endpoints**.
- The `PATCHORCH_SCHEDULE_ID` env override is preserved.
- No changes to the Python engine.
