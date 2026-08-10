# Scrum-Master Task — Phase 1 (Project skeleton + CMake build)

You are the **scrum-master** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 1).

## Your job (you do NOT implement)
1. Review the phase-1 plan in `PLAN.md`.
2. Assign phase 1 to the engineer by updating the `Assigned to` column to
   `senior-engineer-1` and `Status` to `In Progress`.
3. Write the phase-1 tests into `tests/phase1/` (or a clearly named test file). For a
   skeleton/CMake phase, the "tests" are build-verification checks: a script that
   configures the CMake project, builds it, and asserts the expected directory layout and
   that the build produced a verifiable artifact. Make the test script self-contained and
   runnable with a hard timeout.

## Constraints / working rules
- Do NOT implement the phase. Only assign and write tests.
- ALWAYS run commands with a HARD TIMEOUT; run one at a time. No full-filesystem scans.
- Do NOT commit or push.

## Report
Reply `DONE` when phase 1 is assigned and the test script is written.
