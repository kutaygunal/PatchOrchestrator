# Phase 2 Task — Domain model (C++ core)

You are the **senior-engineer-2** agent for PatchOrchestrator. Read `ORCHESTRATION.md`,
`docs/reborn-brief.md`, `docs/working-rules.md`, and `PLAN.md` (phase 2) for context.
Phase 1 (build skeleton) is done and committed.

## Objective (Phase 2)
Implement the plain C++ domain model for the patch control plane: Fleet/Endpoint/Group,
PatchSchedule, MaintenanceWindow, RolloutStage data types plus factory and validation
functions. **No GUI. No I/O. No Qt dependencies** for this phase.

## Deliverables
1. Header + source implementing the domain model. Use the **API contract below exactly**
   (the phase-2 test harness depends on it).
2. Wire a CMake **library target** for the domain model so it builds as part of the phase-1
   build. Add it to the existing root `CMakeLists.txt`.
3. Ensure the domain code compiles cleanly with warnings-as-errors on your compiler.

## API contract (implement exactly)

Header: `src/domain/domain.hpp` (namespace `poc::domain`). Source: `src/domain/domain.cpp`.
CMake library target name: **`poc_domain`** (static library).

```cpp
namespace poc::domain {

enum class PatchState { Pending, Running, Paused, Failed, RolledBack };

struct MaintenanceWindow {
    std::string id;
    std::string start;  // ISO-8601, e.g. "2025-01-01T02:00:00Z"
    std::string end;    // ISO-8601, strictly after start for a valid window
};

struct Endpoint {
    std::string id;
    std::string group_id;
    std::string hostname;
};

struct Group {
    std::string id;
    std::string name;
    std::vector<std::string> endpoint_ids;
};

struct RolloutStage {
    std::string id;
    int order = 0;                      // sequence; lower runs first
    std::vector<std::string> group_ids; // groups patched in this stage
};

struct PatchSchedule {
    std::string id;
    std::string package;
    std::string group_id;               // target group (top-level)
    MaintenanceWindow window;
    std::vector<RolloutStage> stages;
};

// ---- Factories (throw std::invalid_argument on invalid input) ----
Endpoint   make_endpoint(const std::string& id, const std::string& group_id,
                         const std::string& hostname);
MaintenanceWindow make_window(const std::string& id, const std::string& start,
                              const std::string& end);
PatchSchedule make_schedule(const std::string& id, const std::string& package,
                            const std::string& group_id, const MaintenanceWindow& window,
                            const std::vector<RolloutStage>& stages);

// ---- Validation (return bool, never throw) ----
bool is_valid(const Endpoint& e);
bool is_valid(const MaintenanceWindow& w);   // non-empty ids, non-empty start/end, start < end
bool is_valid(const RolloutStage& s);        // non-empty id, non-empty group_ids
bool is_valid(const PatchSchedule& s);       // non-empty id/package/group_id, valid window,
                                             // at least one valid stage

} // namespace poc::domain
```

Validation rules (must match the tests):
- `Endpoint`: valid iff `id` and `group_id` are non-empty.
- `MaintenanceWindow`: valid iff `id`, `start`, `end` non-empty **and** `start < end`
  (lexicographic comparison of the ISO strings is acceptable).
- `RolloutStage`: valid iff `id` non-empty **and** `group_ids` non-empty.
- `PatchSchedule`: valid iff `id`, `package`, `group_id` non-empty, the window is valid,
  **and** there is at least one stage that is valid.
- Factories throw `std::invalid_argument` when constructing an invalid object (e.g. an
  empty id). Include `<stdexcept>` and `<string>`/`<vector>`.

## Constraints / working rules
- ALWAYS run build/test commands with a HARD TIMEOUT, one at a time (e.g.
  `timeout 300 cmake --build build --target poc_domain`).
- Never run a full-filesystem scan (no `find /`).
- Do NOT commit or push — that is the devops agent's job.

## Definition of done
`poc_domain` builds cleanly and the phase-2 verification script in `tests/phase2/` PASSES.

## Report
Reply `DONE` on success or a concise error on failure.
