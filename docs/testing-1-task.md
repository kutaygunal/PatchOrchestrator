# Testing Task — Phase 1 (Project skeleton + CMake build)

You are the **testing-1** agent for PatchOrchestrator. Read `docs/working-rules.md`.

## Your job
Run the phase-1 test script written by scrum-master:

```
timeout 300 bash tests/phase1/verify_build.sh
```

Run it ONE AT A TIME with a hard timeout. Do NOT run any full-filesystem scan.

## Decision
- If the script passes (exit 0): reply `PASS`.
- If it fails: reply `FAIL` followed by a concise error summary.

Do NOT commit or push.
