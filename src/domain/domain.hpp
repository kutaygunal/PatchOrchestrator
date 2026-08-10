// PatchOrchestrator — plain C++ domain model for the patch control plane.
//
// Fleet/Endpoint/Group, PatchSchedule, MaintenanceWindow, RolloutStage data
// types plus factory and validation functions. No GUI, no I/O, no Qt
// dependencies. Namespace: poc::domain.

#pragma once

#include <string>
#include <vector>

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
Endpoint make_endpoint(const std::string& id, const std::string& group_id,
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
