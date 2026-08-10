# Phase 3 simulation-engine verification suite (scrum-master authored).
# Asserts the `engine` API contract, the state machine, determinism, and
# progress/failure/rollback modeling. Run via tests/phase3/run_tests.sh.

import engine


def _fleet(count=3, failure_rate=0.1):
    return [engine.Endpoint(f"ep-{i}", failure_rate=failure_rate) for i in range(count)]


def test_states_constants():
    assert engine.STATES == ("pending", "running", "paused", "failed",
                             "rolled_back", "succeeded")


def test_initial_state():
    eps = _fleet()
    for e in eps:
        assert e.state == "pending"
        assert e.progress == 0.0
        assert e.failed is False


def test_start():
    r = engine.Rollout(_fleet())
    r.start()
    assert r.running is True
    for e in r.endpoints:
        assert e.state == "running"


def test_progress_advances_on_tick():
    r = engine.Rollout(_fleet(failure_rate=0.0))
    r.start()
    r.tick(steps=1)
    for e in r.endpoints:
        assert e.progress >= 1.0


def test_pause_freezes_progress():
    r = engine.Rollout(_fleet(failure_rate=0.0))
    r.start()
    r.tick(steps=2)
    r.pause()
    assert r.paused is True
    frozen = [e.progress for e in r.endpoints]
    r.tick(steps=5)
    for e, p in zip(r.endpoints, frozen):
        assert e.progress == p, "paused rollout must not advance progress"


def test_resume_resumes():
    r = engine.Rollout(_fleet(failure_rate=0.0))
    r.start()
    r.pause()
    r.resume()
    assert r.paused is False
    before = r.endpoints[0].progress
    r.tick(steps=1)
    assert r.endpoints[0].progress > before


def test_success():
    r = engine.Rollout(_fleet(2, failure_rate=0.0))
    r.start()
    r.simulate()
    for e in r.endpoints:
        assert e.state == "succeeded"
        assert e.progress == 100.0


def test_failure_modeling():
    r = engine.Rollout([engine.Endpoint("ep-f", failure_rate=1.0)])
    r.start()
    r.tick(steps=1)
    assert r.endpoints[0].state == "failed"
    assert r.endpoints[0].failed is True


def test_progress_bounds():
    r = engine.Rollout(_fleet(1, failure_rate=0.0))
    r.start()
    for _ in range(300):
        r.tick(steps=1)
    for e in r.endpoints:
        assert 0.0 <= e.progress <= 100.0
        assert e.state == "succeeded"


def test_deterministic_same_seed():
    def run():
        eps = _fleet(8, failure_rate=0.35)
        r = engine.Rollout(eps, seed=12345)
        r.simulate()
        return [(e.id, e.state, round(e.progress, 3)) for e in r.endpoints]

    assert run() == run(), "same seed must produce identical rollout results"


def test_different_seed_is_valid_run():
    def run(seed):
        r = engine.Rollout(_fleet(8, failure_rate=0.35), seed=seed)
        r.simulate()
        return [(e.id, e.state, round(e.progress, 3)) for e in r.endpoints]

    r1 = run(1)
    r2 = run(2)
    for result in (r1, r2):
        for _id, state, progress in result:
            assert state in engine.STATES
            assert state in ("succeeded", "failed"), "simulate must leave terminal states"
            assert 0.0 <= progress <= 100.0


def test_rollback():
    eps = _fleet(4, failure_rate=0.3)
    r = engine.Rollout(eps, seed=7)
    r.start()
    r.simulate()
    succeeded_before = [e for e in r.endpoints if e.state == "succeeded"]
    r.rollback()
    assert r.rolled_back is True
    for e in r.endpoints:
        if e.state == "succeeded":
            continue
        assert e.state == "rolled_back"
    # succeeded endpoints must not be rolled back
    assert len(succeeded_before) == sum(
        1 for e in r.endpoints if e.state == "succeeded")


def test_simulate_terminates():
    r = engine.Rollout(_fleet(10, failure_rate=0.2), seed=99)
    r.start()
    r.simulate()
    for e in r.endpoints:
        assert e.state in ("succeeded", "failed", "rolled_back")
