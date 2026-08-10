# Phase 4 Task — Simulation engine unit tests

You are the **senior-engineer-4** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 4) for context.
Phases 1–3 are committed. The Python engine (`python/engine.py`) is implemented and its
API contract is fixed — do NOT change it.

## Objective (Phase 4)
Write a thorough `pytest` suite for the simulation engine covering progress, failures,
pause/resume, rollback, and boundary cases.

## Deliverables
1. `tests/phase4/test_engine.py` (and any helper modules you need) — a `pytest` suite that
   imports the engine as `import engine` (the runner sets `PYTHONPATH` to `python/`).
2. Cover at least the following areas (the phase-4 runner runs the whole suite):
   - **Progress:** running endpoints advance; progress is bounded to `[0.0, 100.0]`; an
     endpoint reaching `100.0` becomes `"succeeded"` and stops advancing.
   - **Failures:** an endpoint with `failure_rate=1.0` fails on its first tick; a failed
     endpoint has `failed=True` and state `"failed"`; a `failure_rate=0.0` endpoint never
     fails.
   - **Pause/resume:** `pause()` freezes progress; `resume()` resumes advancement; a paused
     rollout does not advance on `tick()`.
   - **Rollback:** `rollback()` sets every non-`"succeeded"` endpoint to `"rolled_back"`;
     `"succeeded"` endpoints stay `"succeeded"`; `rolled_back=True` is set.
   - **Determinism:** same endpoints + same seed → identical results (reproducibility).
   - **Boundary cases:** empty fleet; `failure_rate` at 0.0 and 1.0; `steps` > 1 in `tick`;
     `simulate()` on an already-terminal rollout; progress never exceeds 100.0.
3. Do NOT modify `python/engine.py`.

## Constraints / working rules
- ALWAYS run test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 180 bash tests/phase4/run_tests.sh`).
- On this machine `python3` is a Windows Store alias WITHOUT pytest; the `python`
  interpreter (3.11) HAS pytest. Use `python` (not `python3`) when running pytest.
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`tests/phase4/run_tests.sh` (the scrum-master runner) exits 0 with all tests passing.

## Report
Reply `DONE` on success or a concise error on failure.
