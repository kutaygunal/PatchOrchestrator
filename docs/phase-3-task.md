# Phase 3 Task — Python simulation engine

You are the **senior-engineer-3** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 3) for context.
Phase 1 (build skeleton) is done and committed. Phase 3 runs IN PARALLEL with phase 2.

## Objective (Phase 3)
Implement a deterministic, seeded Python simulation engine modeling endpoint patch state:
`pending → running → paused → running → failed / rolled_back / succeeded`. Model progress,
failures, and rollback. **No network, no GUI, no .NET coupling** in this phase.

## Deliverables
1. `python/engine.py` implementing the **API contract below exactly** (the phase-3 test
   suite depends on it).
2. The engine must be **deterministic**: given the same config and seed, it always produces
   the same result.

## API contract (implement exactly)

File: `python/engine.py`. Importable as `import engine` (the test harness sets
`PYTHONPATH` to the `python/` directory).

```python
# Constants (module-level)
STATES = ("pending", "running", "paused", "failed", "rolled_back", "succeeded")

class Endpoint:
    def __init__(self, endpoint_id, failure_rate=0.1):
        # endpoint_id: str (unique), failure_rate: float in [0.0, 1.0]
        self.id = endpoint_id
        self.failure_rate = failure_rate
        self.state = "pending"
        self.progress = 0.0        # float in [0.0, 100.0]
        self.failed = False

class Rollout:
    def __init__(self, endpoints, seed=42):
        # endpoints: list[Endpoint]; seed: int
        self.endpoints = list(endpoints)
        self._rng = random.Random(seed)   # deterministic
        self.running = False
        self.paused = False
        self.rolled_back = False

    def start(self):                 # pending -> running
    def pause(self):                 # running -> paused (progress frozen)
    def resume(self):                # paused -> running
    def rollback(self):              # running/paused/failed -> rolled_back
    def tick(self, steps=1):         # advance running endpoints deterministically
    def simulate(self):              # run to completion (all endpoints terminal)
```

Determinism & modelling rules (must match the tests):
- `tick()` uses ONLY `self._rng` for randomness, so a fixed seed reproduces results exactly.
- On each `tick`, a running endpoint advances progress by `1.0 * step_count` unless a
  random draw `self._rng.random() < self.failure_rate` fails it: on failure the endpoint
  becomes `"failed"` and `failed=True`.
- `progress` never exceeds `100.0`. When `progress >= 100.0` the endpoint becomes
  `"succeeded"` and no longer advances.
- `pause()` freezes all running endpoints (progress unchanged during pause). `resume()`
  unfreezes them.
- `rollback()` sets every non-`"succeeded"` endpoint to `"rolled_back"` and sets
  `self.rolled_back = True`. `"succeeded"` endpoints stay `"succeeded"`.
- `simulate()` keeps calling `tick(1)` until every endpoint is in a terminal state
  (`"succeeded"`, `"failed"`, or `"rolled_back"`), then returns `self`.

## Constraints / working rules
- ALWAYS run test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 120 python -m pytest tests/phase3`).
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`python/engine.py` imports cleanly and the phase-3 pytest suite in `tests/phase3/` PASSES.

## Report
Reply `DONE` on success or a concise error on failure.
