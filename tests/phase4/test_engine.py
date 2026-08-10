"""Phase 4 pytest suite for the PatchOrchestrator simulation engine.

Imports the engine as ``import engine`` (the runner sets ``PYTHONPATH`` to
``python/``).  Covers progress, failures, pause/resume, rollback,
determinism, and boundary cases.  Does NOT modify ``python/engine.py``.
"""

import pytest

import engine
from engine import Endpoint, Rollout


def make_fleet(n, failure_rate=0.0, seed=42):
    """Build a fleet of ``n`` endpoints with a shared failure rate."""
    return [Endpoint(f"ep-{i}", failure_rate=failure_rate) for i in range(n)]


# ---------------------------------------------------------------------------
# Progress
# ---------------------------------------------------------------------------

class TestProgress:
    def test_running_endpoints_advance(self):
        fleet = make_fleet(3, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        for ep in fleet:
            assert ep.state == "running"
            assert ep.progress > 0.0

    def test_progress_bounded_upper(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        for _ in range(500):
            r.tick(1)
        for ep in fleet:
            assert ep.progress <= 100.0

    def test_progress_bounded_lower(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        for ep in fleet:
            assert ep.progress >= 0.0

    def test_reaching_100_becomes_succeeded_and_stops(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        # Advance well past 100 ticks.
        for _ in range(200):
            r.tick(1)
        ep = fleet[0]
        assert ep.progress == 100.0
        assert ep.state == "succeeded"
        # Further ticks must not change a succeeded endpoint.
        before = ep.progress
        r.tick(1)
        assert ep.progress == before
        assert ep.state == "succeeded"


# ---------------------------------------------------------------------------
# Failures
# ---------------------------------------------------------------------------

class TestFailures:
    def test_failure_rate_1_fails_on_first_tick(self):
        fleet = make_fleet(1, failure_rate=1.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        ep = fleet[0]
        assert ep.failed is True
        assert ep.state == "failed"

    def test_failed_endpoint_has_failed_flag_and_state(self):
        fleet = make_fleet(1, failure_rate=1.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        ep = fleet[0]
        assert ep.failed is True
        assert ep.state == "failed"

    def test_failure_rate_0_never_fails(self):
        fleet = make_fleet(5, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        for _ in range(200):
            r.tick(1)
        for ep in fleet:
            assert ep.failed is False
            assert ep.state in ("running", "succeeded")

    def test_failed_endpoint_stops_advancing(self):
        fleet = make_fleet(1, failure_rate=1.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        ep = fleet[0]
        progress_after_fail = ep.progress
        r.tick(1)
        assert ep.progress == progress_after_fail
        assert ep.state == "failed"


# ---------------------------------------------------------------------------
# Pause / resume
# ---------------------------------------------------------------------------

class TestPauseResume:
    def test_pause_freezes_progress(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        frozen = fleet[0].progress
        r.pause()
        r.tick(1)
        assert fleet[0].progress == frozen

    def test_paused_rollout_does_not_advance_on_tick(self):
        fleet = make_fleet(3, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        frozen = [ep.progress for ep in fleet]
        r.pause()
        r.tick(1)
        assert [ep.progress for ep in fleet] == frozen

    def test_resume_resumes_advancement(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        r.pause()
        frozen = fleet[0].progress
        r.resume()
        r.tick(1)
        assert fleet[0].progress > frozen

    def test_pause_sets_paused_flag(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.pause()
        assert r.paused is True
        assert r.running is False

    def test_resume_clears_paused_flag(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.pause()
        r.resume()
        assert r.paused is False
        assert r.running is True


# ---------------------------------------------------------------------------
# Rollback
# ---------------------------------------------------------------------------

class TestRollback:
    def test_rollback_sets_non_succeeded_to_rolled_back(self):
        fleet = make_fleet(4, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        # Advance some endpoints to succeeded, leave others running.
        for _ in range(50):
            r.tick(1)
        r.rollback()
        for ep in fleet:
            if ep.state != "succeeded":
                assert ep.state == "rolled_back"

    def test_succeeded_stay_succeeded(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        for _ in range(200):
            r.tick(1)
        assert fleet[0].state == "succeeded"
        r.rollback()
        assert fleet[0].state == "succeeded"

    def test_rollback_sets_rolled_back_flag(self):
        fleet = make_fleet(2, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        r.rollback()
        assert r.rolled_back is True

    def test_rollback_clears_running_and_paused(self):
        fleet = make_fleet(2, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.pause()
        r.rollback()
        assert r.running is False
        assert r.paused is False

    def test_rollback_handles_failed_endpoints(self):
        fleet = make_fleet(2, failure_rate=1.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        assert all(ep.state == "failed" for ep in fleet)
        r.rollback()
        assert all(ep.state == "rolled_back" for ep in fleet)


# ---------------------------------------------------------------------------
# Determinism
# ---------------------------------------------------------------------------

class TestDeterminism:
    def test_same_seed_same_result(self):
        def run(seed):
            fleet = make_fleet(10, failure_rate=0.3)
            r = Rollout(fleet, seed=seed)
            r.simulate()
            return [(ep.state, ep.progress, ep.failed) for ep in fleet]

        assert run(7) == run(7)

    def test_different_seed_can_differ(self):
        def run(seed):
            fleet = make_fleet(10, failure_rate=0.3)
            r = Rollout(fleet, seed=seed)
            r.simulate()
            return [(ep.state, ep.progress, ep.failed) for ep in fleet]

        # Not guaranteed to differ, but with 10 endpoints and 0.3 rate it
        # overwhelmingly will; guard against a trivially broken RNG.
        assert run(1) != run(2) or run(1) != run(3)

    def test_same_seed_same_tick_sequence(self):
        def run(seed):
            fleet = make_fleet(5, failure_rate=0.2)
            r = Rollout(fleet, seed=seed)
            r.start()
            snapshots = []
            for _ in range(20):
                r.tick(1)
                snapshots.append([(ep.state, ep.progress) for ep in fleet])
            return snapshots

        assert run(99) == run(99)


# ---------------------------------------------------------------------------
# Boundary cases
# ---------------------------------------------------------------------------

class TestBoundary:
    def test_empty_fleet(self):
        r = Rollout([], seed=1)
        r.start()
        r.tick(1)
        assert r.endpoints == []
        r.simulate()
        assert r.endpoints == []

    def test_failure_rate_0_boundary(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        for _ in range(200):
            r.tick(1)
        assert fleet[0].state == "succeeded"
        assert fleet[0].failed is False

    def test_failure_rate_1_boundary(self):
        fleet = make_fleet(1, failure_rate=1.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(1)
        assert fleet[0].state == "failed"
        assert fleet[0].failed is True

    def test_tick_with_steps_greater_than_one(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(steps=10)
        assert fleet[0].progress == 10.0

    def test_tick_steps_capped_at_100(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        r.tick(steps=500)
        assert fleet[0].progress == 100.0
        assert fleet[0].state == "succeeded"

    def test_simulate_on_already_terminal_rollout(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.simulate()
        assert fleet[0].state == "succeeded"
        # Calling simulate again on a terminal rollout must not error.
        r.simulate()
        assert fleet[0].state == "succeeded"

    def test_progress_never_exceeds_100(self):
        fleet = make_fleet(1, failure_rate=0.0)
        r = Rollout(fleet, seed=1)
        r.start()
        for _ in range(1000):
            r.tick(1)
        assert fleet[0].progress == 100.0

    def test_simulate_runs_to_completion(self):
        fleet = make_fleet(6, failure_rate=0.5)
        r = Rollout(fleet, seed=5)
        r.simulate()
        for ep in fleet:
            assert ep.state in ("succeeded", "failed", "rolled_back")
