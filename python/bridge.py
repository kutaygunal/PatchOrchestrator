"""JSON bridge between the .NET API and the Python simulation engine.

Reads a single JSON object from stdin, runs ``engine.Rollout(...).simulate()``,
and prints a single JSON object to stdout.

Contract (stdin):
    {"endpoints":[{"id":"ep-1","failure_rate":0.1}, ...],"seed":42}

Contract (stdout):
    {"endpoints":[{"id":"ep-1","state":"succeeded","progress":100.0}, ...],
     "rolled_back":false}

Structured logging is written to stderr (never stdout, so the JSON contract
stays clean): each request is logged with a timestamp + level + message, and
errors surface as a JSON ``{"error": ...}`` object on stderr with a non-zero
exit code.

Runnable as ``python bridge.py`` with ``PYTHONPATH`` pointing at ``python/``
so ``import engine`` resolves. Deterministic for a fixed seed.
"""

import datetime
import json
import sys

import engine


def _log(level: str, message: str) -> None:
    """Write a structured [ISO-8601] LEVEL message to stderr."""
    ts = datetime.datetime.now(datetime.timezone.utc).isoformat()
    print(f"{ts} [{level}] {message}", file=sys.stderr, flush=True)


def _error(message: str) -> int:
    """Surface a structured error to stderr and return a failure exit code."""
    _log("ERROR", message)
    print(json.dumps({"error": message}), file=sys.stderr, flush=True)
    return 1


def main() -> int:
    _log("INFO", "bridge invoked")
    raw = sys.stdin.read()
    if not raw.strip():
        return _error("no input on stdin")

    try:
        request = json.loads(raw)
    except json.JSONDecodeError as exc:
        return _error(f"invalid JSON: {exc}")

    if not isinstance(request, dict):
        return _error("request must be a JSON object")

    endpoints_spec = request.get("endpoints")
    if not isinstance(endpoints_spec, list):
        return _error("'endpoints' must be an array")

    endpoints = []
    for ep in endpoints_spec:
        try:
            ep_id = ep["id"]
        except (TypeError, KeyError) as exc:
            return _error(f"endpoint entry missing 'id': {exc}")
        try:
            failure_rate = float(ep.get("failure_rate", 0.0))
        except (TypeError, ValueError) as exc:
            return _error(f"endpoint '{ep_id}' has an invalid 'failure_rate': {exc}")
        endpoints.append(engine.Endpoint(ep_id, failure_rate))

    try:
        seed = int(request.get("seed", 42))
    except (TypeError, ValueError) as exc:
        return _error(f"invalid 'seed': {exc}")

    rollout = engine.Rollout(endpoints, seed).simulate()

    result = {
        "endpoints": [
            {"id": ep.id, "state": ep.state, "progress": float(ep.progress)}
            for ep in rollout.endpoints
        ],
        "rolled_back": bool(rollout.rolled_back),
    }
    _log("INFO", f"bridge produced result for {len(result['endpoints'])} endpoint(s)")
    print(json.dumps(result))
    return 0


if __name__ == "__main__":
    sys.exit(main())

