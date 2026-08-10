"""Deterministic, seeded Python simulation engine for PatchOrchestrator.

Models endpoint patch state: pending -> running -> paused -> running ->
failed / rolled_back / succeeded.  No network, no GUI, no .NET coupling.

Importable as ``import engine`` (the test harness sets ``PYTHONPATH`` to the
``python/`` directory).  Deterministic: given the same config and seed, it
always produces the same result.
"""

import datetime
import random
import sys

# Module-level constants (API contract).
STATES = ("pending", "running", "paused", "failed", "rolled_back", "succeeded")

# Terminal states: the rollout stops advancing an endpoint once it reaches one.
_TERMINAL = ("succeeded", "failed", "rolled_back")


def _log(level: str, message: str) -> None:
    """Structured [ISO-8601] LEVEL message to stderr (never stdout)."""
    ts = datetime.datetime.now(datetime.timezone.utc).isoformat()
    print(f"{ts} [{level}] {message}", file=sys.stderr, flush=True)


class Endpoint:
    """A single endpoint's patch state and progress."""

    def __init__(self, endpoint_id, failure_rate=0.1):
        # endpoint_id: str (unique), failure_rate: float in [0.0, 1.0]
        self.id = endpoint_id
        self.failure_rate = failure_rate
        self.state = "pending"
        self.progress = 0.0  # float in [0.0, 100.0]
        self.failed = False


class Rollout:
    """A deterministic, seeded rollout over a fleet of endpoints."""

    def __init__(self, endpoints, seed=42):
        # endpoints: list[Endpoint]; seed: int
        self.endpoints = list(endpoints)
        self._rng = random.Random(seed)  # deterministic
        self.running = False
        self.paused = False
        self.rolled_back = False

    def start(self):
        """pending -> running for every endpoint."""
        for ep in self.endpoints:
            if ep.state == "pending":
                ep.state = "running"
        self.running = True
        self.paused = False
        _log("INFO", f"rollout started ({len(self.endpoints)} endpoint(s))")

    def pause(self):
        """running -> paused (progress frozen)."""
        if self.running and not self.paused:
            self.paused = True
            self.running = False
            _log("INFO", "rollout paused")

    def resume(self):
        """paused -> running."""
        if self.paused:
            self.paused = False
            self.running = True
            _log("INFO", "rollout resumed")

    def rollback(self):
        """running/paused/failed -> rolled_back.  Succeeded stay succeeded."""
        for ep in self.endpoints:
            if ep.state != "succeeded":
                ep.state = "rolled_back"
        self.rolled_back = True
        self.running = False
        self.paused = False
        _log("WARN", "rollout rolled back")

    def tick(self, steps=1):
        """Advance running endpoints deterministically.

        While paused, progress is frozen (no draws, no advancement).
        Uses ONLY ``self._rng`` for randomness so a fixed seed reproduces
        results exactly.
        """
        if self.paused:
            return self
        for ep in self.endpoints:
            if ep.state != "running":
                continue
            # One random draw per running endpoint per tick.
            if self._rng.random() < ep.failure_rate:
                ep.state = "failed"
                ep.failed = True
                continue
            ep.progress = min(100.0, ep.progress + 1.0 * steps)
            if ep.progress >= 100.0:
                ep.progress = 100.0
                ep.state = "succeeded"
        return self

    def simulate(self):
        """Run to completion (all endpoints terminal), then return self."""
        if not self.running:
            self.start()
        while not all(ep.state in _TERMINAL for ep in self.endpoints):
            self.tick(1)
        _log("INFO", "rollout simulation complete")
        return self
