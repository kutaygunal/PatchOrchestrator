// PatchOrchestrator — domain model implementation.
//
// Implements the factory and validation functions declared in domain.hpp.

#include "domain/domain.hpp"

#include <stdexcept>

namespace poc::domain {

// ---- Validation ------------------------------------------------------------

bool is_valid(const Endpoint& e) {
    return !e.id.empty() && !e.group_id.empty();
}

bool is_valid(const MaintenanceWindow& w) {
    // Lexicographic comparison of the ISO-8601 strings is acceptable per the
    // contract (ISO-8601 ordering matches lexicographic ordering).
    return !w.id.empty() && !w.start.empty() && !w.end.empty() && w.start < w.end;
}

bool is_valid(const RolloutStage& s) {
    return !s.id.empty() && !s.group_ids.empty();
}

bool is_valid(const PatchSchedule& s) {
    if (s.id.empty() || s.package.empty() || s.group_id.empty()) {
        return false;
    }
    if (!is_valid(s.window)) {
        return false;
    }
    for (const RolloutStage& stage : s.stages) {
        if (is_valid(stage)) {
            return true;  // at least one valid stage
        }
    }
    return false;
}

// ---- Factories (throw std::invalid_argument on invalid input) --------------

Endpoint make_endpoint(const std::string& id, const std::string& group_id,
                       const std::string& hostname) {
    Endpoint e{id, group_id, hostname};
    if (!is_valid(e)) {
        throw std::invalid_argument("make_endpoint: endpoint must have a non-empty id and group_id");
    }
    return e;
}

MaintenanceWindow make_window(const std::string& id, const std::string& start,
                              const std::string& end) {
    MaintenanceWindow w{id, start, end};
    if (!is_valid(w)) {
        throw std::invalid_argument(
            "make_window: window needs non-empty id/start/end and start must precede end");
    }
    return w;
}

PatchSchedule make_schedule(const std::string& id, const std::string& package,
                            const std::string& group_id, const MaintenanceWindow& window,
                            const std::vector<RolloutStage>& stages) {
    PatchSchedule s{id, package, group_id, window, stages};
    if (!is_valid(s)) {
        throw std::invalid_argument(
            "make_schedule: schedule needs non-empty id/package/group_id, a valid window, "
            "and at least one valid stage");
    }
    return s;
}

} // namespace poc::domain
