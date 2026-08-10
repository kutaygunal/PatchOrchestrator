# Orchestration Manual

> Working rules are single-sourced in `_shared/working-rules.md`. Follow them. See the
> Context guard section below and the `reborn` skill for handoff.

This project is driven by a multi-agent loop. Roles:

| Agent | Duty |
|-------|------|
| `planner` | Produces/updates the phased plan in a `PLAN*.md` tracker. |
| `scrum-master` | Reviews the plan, assigns the current phase to an engineer, writes the phase tests. Does NOT implement. |
| `senior-engineer-<phase>` | Implements the current phase. Re-spawned per phase. |
| `testing-<phase>` | Runs the phase tests. FAIL -> engineer; PASS -> devops. |
| `devops-<phase>` | Commits and pushes. |
| `orchestrator` | Coordinates the loop, closes non-essential agents between phases. |

Workflow loop per phase:
1. Planner produces/updates the plan.
2. Scrum-master reviews, assigns the phase, writes the phase tests.
3. Engineer implements and reports "done".
4. Testing runs the phase tests. FAIL -> engineer; PASS -> devops.
5. Devops commits and pushes, reports "phase finished".
6. Orchestrator closes subagents except scrum-master and planner.
7. Orchestrator informs scrum-master and planner the phase is done.
8. Scrum-master starts the next loop -> assigns the next phase to a fresh engineer.
9. Loop until all phases in the tracker are complete.

## Context guard

The orchestrator tracks its context usage in `docs/context-log.json`. At **~30M tokens**
warn the user; at **~45M tokens** STOP starting new phases and suggest the user run the
**`reborn` skill** so a fresh-context successor can take over with full project state.
