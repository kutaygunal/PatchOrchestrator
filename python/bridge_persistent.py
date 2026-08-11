"""Persistent JSON bridge between the .NET API and the Python simulation engine.

Unlike ``bridge.py`` (which handles a single request and exits), this script
keeps ONE long-lived subprocess alive across many requests so live engine
state (pause/resume/rollback/tick) can be mutated across calls. It reads one
JSON request per line from stdin and writes one JSON response per line to
stdout.

Request contract (one JSON object per line on stdin):
    {"cmd":"run","endpoints":[{"id":"ep-1","failure_rate":0.1},...],"seed":42}
    {"cmd":"start","endpoints":[...],"seed":42}
    {"cmd":"pause"}
    {"cmd":"resume"}
    {"cmd":"rollback"}
    {"cmd":"tick","steps":1}
    {"cmd":"state"}
    {"cmd":"shutdown"}

Response contract (one JSON object per line on stdout):
    {"ok":true,"endpoints":[{"id":"ep-1","state":"succeeded","progress":100.0},...],
     "rolled_back":false}
    {"ok":false,"error":"..."}

Structured logging is written to stderr (never stdout, so the JSON contract
stays clean). Deterministic for a fixed seed.

Runnable as ``python bridge_persistent.py`` with ``PYTHONPATH`` pointing at
``python/`` so ``import engine`` resolves.
"""

import datetime
import json
import sys

import engine


def _log(level: str, message: str) -> None:
    """Write a structured [ISO-8601] LEVEL message to stderr."""
    ts = datetime.datetime.now(datetime.timezone.utc).isoformat()
    print(f"{ts} [{level}] {message}", file=sys.stderr, flush=True)


def _respond(obj) -> None:
    """Write a single JSON response line to stdout."""
    print(json.dumps(obj), flush=True)


def _state(rollout) -> dict:
    """Snapshot the current rollout state as a JSON-serializable dict."""
    return {
        "endpoints": [
            {"id": ep.id, "state": ep.state, "progress": float(ep.progress)}
            for ep in rollout.endpoints
        ],
        "rolled_back": bool(rollout.rolled_back),
    }


def _build_rollout(req) -> engine.Rollout:
    """Build a Rollout from a request dict (shared by run/start)."""
    endpoints_spec = req.get("endpoints")
    if not isinstance(endpoints_spec, list):
        raise ValueError("'endpoints' must be an array")

    endpoints = []
    for ep in endpoints_spec:
        try:
            ep_id = ep["id"]
        except (TypeError, KeyError) as exc:
            raise ValueError(f"endpoint entry missing 'id': {exc}") from exc
        try:
            failure_rate = float(ep.get("failure_rate", 0.0))
        except (TypeError, ValueError) as exc:
            raise ValueError(f"endpoint '{ep_id}' has an invalid 'failure_rate': {exc}") from exc
        endpoints.append(engine.Endpoint(ep_id, failure_rate))

    try:
        seed = int(req.get("seed", 42))
    except (TypeError, ValueError) as exc:
        raise ValueError(f"invalid 'seed': {exc}") from exc

    return engine.Rollout(endpoints, seed)


def _handle(req, rollout):
    """Dispatch a single request. Returns the (possibly new) rollout."""
    cmd = req.get("cmd")
    if cmd == "run":
        # One-shot "simulate to completion" (used by the read-only dashboard's
        # poll loop). This must NOT replace the live session `rollout` that
        # "start" established and that pause/resume/rollback/tick mutate in
        # place — this subprocess is shared by both Qt apps (control panel and
        # dashboard) through one persistent bridge, so reassigning `rollout`
        # here used to silently reset/clobber the control panel's live rollout
        # on every dashboard poll tick. Use a throwaway rollout instead and
        # leave the session's `rollout` (if any) untouched.
        scratch = _build_rollout(req).simulate()
        _respond({"ok": True, **_state(scratch)})
    elif cmd == "start":
        rollout = _build_rollout(req)
        rollout.start()
        _respond({"ok": True, **_state(rollout)})
    elif cmd == "pause":
        if rollout is None:
            _respond({"ok": False, "error": "no active rollout"})
        else:
            rollout.pause()
            _respond({"ok": True, **_state(rollout)})
    elif cmd == "resume":
        if rollout is None:
            _respond({"ok": False, "error": "no active rollout"})
        else:
            rollout.resume()
            _respond({"ok": True, **_state(rollout)})
    elif cmd == "rollback":
        if rollout is None:
            _respond({"ok": False, "error": "no active rollout"})
        else:
            rollout.rollback()
            _respond({"ok": True, **_state(rollout)})
    elif cmd == "tick":
        if rollout is None:
            _respond({"ok": False, "error": "no active rollout"})
        else:
            try:
                steps = int(req.get("steps", 1))
            except (TypeError, ValueError) as exc:
                _respond({"ok": False, "error": f"invalid 'steps': {exc}"})
                return rollout
            rollout.tick(steps)
            _respond({"ok": True, **_state(rollout)})
    elif cmd == "state":
        if rollout is None:
            _respond({"ok": False, "error": "no active rollout"})
        else:
            _respond({"ok": True, **_state(rollout)})
    elif cmd == "shutdown":
        _respond({"ok": True, "shutdown": True})
        _log("INFO", "persistent bridge shutting down")
        return None
    else:
        _respond({"ok": False, "error": f"unknown command: {cmd}"})
    return rollout


def main() -> int:
    _log("INFO", "persistent bridge started")
    rollout = None
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        try:
            req = json.loads(line)
        except json.JSONDecodeError as exc:
            _respond({"ok": False, "error": f"invalid JSON: {exc}"})
            continue
        if not isinstance(req, dict):
            _respond({"ok": False, "error": "request must be a JSON object"})
            continue
        try:
            rollout = _handle(req, rollout)
        except Exception as exc:  # noqa: BLE001 - surface any engine error
            _respond({"ok": False, "error": str(exc)})
        if rollout is None and req.get("cmd") == "shutdown":
            return 0
    _log("INFO", "persistent bridge stdin closed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
