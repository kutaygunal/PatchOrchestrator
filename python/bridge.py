"""JSON bridge between the .NET API and the Python simulation engine.

Reads a single JSON object from stdin, runs ``engine.Rollout(...).simulate()``,
and prints a single JSON object to stdout.

Contract (stdin):
    {"endpoints":[{"id":"ep-1","failure_rate":0.1}, ...],"seed":42}

Contract (stdout):
    {"endpoints":[{"id":"ep-1","state":"succeeded","progress":100.0}, ...],
     "rolled_back":false}

Runnable as ``python bridge.py`` with ``PYTHONPATH`` pointing at ``python/``
so ``import engine`` resolves. Deterministic for a fixed seed.
"""

import json
import sys

import engine


def main() -> int:
    raw = sys.stdin.read()
    if not raw.strip():
        print(json.dumps({"error": "no input on stdin"}), file=sys.stderr)
        return 1

    try:
        request = json.loads(raw)
    except json.JSONDecodeError as exc:
        print(json.dumps({"error": f"invalid JSON: {exc}"}), file=sys.stderr)
        return 1

    endpoints = [
        engine.Endpoint(ep["id"], float(ep.get("failure_rate", 0.0)))
        for ep in request.get("endpoints", [])
    ]
    seed = int(request.get("seed", 42))

    rollout = engine.Rollout(endpoints, seed).simulate()

    result = {
        "endpoints": [
            {"id": ep.id, "state": ep.state, "progress": float(ep.progress)}
            for ep in rollout.endpoints
        ],
        "rolled_back": bool(rollout.rolled_back),
    }
    print(json.dumps(result))
    return 0


if __name__ == "__main__":
    sys.exit(main())
