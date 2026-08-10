# Working Rules (single source of truth)

These rules prevent hangs, freezes, and runaway work. Every orchestrator birth brief,
`ORCHESTRATION.md`, and subagent must follow them. Change them HERE once — the birth briefs
reference this file instead of duplicating it.

## Anti-hang rules

1. **NEVER run `find /` or any full-filesystem scan.** It hangs. Use bounded `ls -R`/`grep`
   in known trees only (`src/`, `python/`, `tests/`, `docs/`, etc.).
2. **ALWAYS run build/test commands with a HARD TIMEOUT** (e.g. `timeout 300 dotnet test ...`,
   `timeout 300 python -m pytest ...`). Never run a single command unbounded.
3. **Run tests ONE AT A TIME with timeouts**, never a giant combined command that can hang.
4. **Do not spawn long-running background processes.** Every shell command must return.
5. If you suspect a hang/deadlock, **stop and fix it** rather than re-running the same hanging
   test.

## Git / role rules

6. **Do not commit/push from engineer or testing agents** — that is the devops agent's job.
7. **Only the `devops-<phase>` agent commits and pushes.** Enforce this with a pre-push
   check.

## Context rules

8. Track context usage before each new phase.
9. At **~30M tokens**: warn the user that context is running low.
10. At **~45M tokens**: **STOP starting new phases** and suggest running the **`reborn`**
    skill so a fresh-context successor can take over with full project state.

## Delegation rules

11. For any non-trivial task, **write the task to a file** (`docs/<phase>-task.md`) and give
    the agent a SHORT prompt to read and execute it. Long inline prompts are flaky and bloat
    context.
12. **Read agent output selectively** — read only the tail (last 15–20 lines) of an agent's
    reply, never the full transcript, to keep orchestrator context low.
