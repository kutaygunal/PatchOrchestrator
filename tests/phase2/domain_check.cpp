// Phase 2 domain-model verification program (scrum-master authored).
// Compiles against the `poc::domain` implementation and asserts the API
// contract + validation rules. Prints "DOMAIN_CHECK_OK" and exits 0 on
// success; prints an error and exits nonzero on any failed assertion.
//
// Contract under test:
//   * Factories THROW std::invalid_argument on invalid input.
//   * is_valid() returns bool and never throws.

#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <cstdlib>

#include "domain/domain.hpp"   // resolved via -Isrc

using namespace poc::domain;

static int failures = 0;

static void check(bool cond, const std::string& label) {
    if (!cond) {
        std::cerr << "FAIL: " << label << "\n";
        ++failures;
    } else {
        std::cout << "ok: " << label << "\n";
    }
}

// Asserts that `func` throws std::invalid_argument (factory contract).
static void expect_throw(const std::string& label,
                         const std::function<void()>& func) {
    try {
        func();
        check(false, label + " (expected invalid_argument, none thrown)");
    } catch (const std::invalid_argument&) {
        check(true, label);
    } catch (const std::exception& e) {
        check(false, label + " (wrong exception type: " + e.what() + ")");
    }
}

int main() {
    // --- Valid Endpoint ---
    Endpoint e = make_endpoint("ep-1", "grp-1", "host-1");
    check(e.id == "ep-1" && e.group_id == "grp-1" && e.hostname == "host-1",
          "make_endpoint sets fields");
    check(is_valid(e), "valid endpoint accepted");

    // --- Invalid Endpoint: factory throws; is_valid rejects direct construction ---
    expect_throw("make_endpoint throws on empty id",
                 [] { make_endpoint("", "grp", "h"); });
    Endpoint invalid_e{"", "grp", "h"};          // aggregate init, bypasses factory
    check(!is_valid(invalid_e), "invalid endpoint rejected by is_valid");

    // --- Valid window ---
    MaintenanceWindow w = make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    check(is_valid(w), "valid window accepted");

    // --- Invalid window: start>=end -> factory throws; is_valid rejects ---
    expect_throw("make_window throws on start>=end",
                 [] { make_window("w-2", "2025-01-01T04:00:00Z", "2025-01-01T02:00:00Z"); });
    MaintenanceWindow bad_w{"w-2", "2025-01-01T04:00:00Z", "2025-01-01T02:00:00Z"};
    check(!is_valid(bad_w), "start>=end window rejected by is_valid");

    // --- RolloutStage ---
    RolloutStage rs{"stage-1", 1, {"grp-1"}};
    check(is_valid(rs), "valid stage accepted");
    RolloutStage bad_rs{"stage-2", 2, {}};
    check(!is_valid(bad_rs), "stage with no groups rejected");

    // --- PatchSchedule valid ---
    PatchSchedule ps = make_schedule("sch-1", "pkg-v2", "grp-1", w, {rs});
    check(is_valid(ps), "valid schedule accepted");
    check(ps.stages.size() == 1 && ps.stages[0].id == "stage-1", "schedule stores stages");

    // --- Invalid PatchSchedule: factory throws on each invalid case ---
    expect_throw("make_schedule throws on empty id",
                 [&] { make_schedule("", "pkg", "grp", w, {rs}); });
    expect_throw("make_schedule throws on empty package",
                 [&] { make_schedule("s", "", "grp", w, {rs}); });
    expect_throw("make_schedule throws when no stages",
                 [&] { make_schedule("s", "pkg", "grp", w, {}); });
    expect_throw("make_schedule throws on invalid window",
                 [&] { make_schedule("s", "pkg", "grp", bad_w, {rs}); });

    // --- is_valid rejects directly-constructed invalid schedules ---
    PatchSchedule ps_empty_id{"", "pkg", "grp", w, {rs}};
    check(!is_valid(ps_empty_id), "empty schedule id rejected by is_valid");
    PatchSchedule ps_empty_pkg{"s", "", "grp", w, {rs}};
    check(!is_valid(ps_empty_pkg), "empty schedule package rejected by is_valid");
    PatchSchedule ps_no_stages{"s", "pkg", "grp", w, {}};
    check(!is_valid(ps_no_stages), "schedule without stages rejected by is_valid");
    PatchSchedule ps_bad_window{"s", "pkg", "grp", bad_w, {rs}};
    check(!is_valid(ps_bad_window), "schedule with invalid window rejected by is_valid");

    if (failures == 0) {
        std::cout << "DOMAIN_CHECK_OK\n";
        return 0;
    }
    std::cerr << failures << " check(s) failed\n";
    return 1;
}
