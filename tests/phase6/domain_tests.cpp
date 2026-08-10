// Phase 6 — C++ core unit tests (GoogleTest).
//
// Covers the `poc::domain` domain model and its validation rules:
//   * Factories: valid construction sets fields correctly; invalid input
//     throws std::invalid_argument.
//   * Validation: is_valid() returns true for valid objects and false for
//     invalid ones, and never throws.
//   * Domain types: PatchState enum values; RolloutStage.order; PatchSchedule
//     stores its stages.

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

#include "domain/domain.hpp"

using namespace poc::domain;

namespace {

// ---- Factories: valid construction sets fields correctly -------------------

TEST(DomainFactories, MakeEndpointSetsFields) {
    Endpoint e = make_endpoint("ep-1", "grp-1", "host-1");
    EXPECT_EQ(e.id, "ep-1");
    EXPECT_EQ(e.group_id, "grp-1");
    EXPECT_EQ(e.hostname, "host-1");
}

TEST(DomainFactories, MakeWindowSetsFields) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    EXPECT_EQ(w.id, "w-1");
    EXPECT_EQ(w.start, "2025-01-01T02:00:00Z");
    EXPECT_EQ(w.end, "2025-01-01T04:00:00Z");
}

TEST(DomainFactories, MakeScheduleSetsFieldsAndStoresStages) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    RolloutStage rs{"stage-1", 1, {"grp-1"}};
    PatchSchedule ps = make_schedule("sch-1", "pkg-v2", "grp-1", w, {rs});

    EXPECT_EQ(ps.id, "sch-1");
    EXPECT_EQ(ps.package, "pkg-v2");
    EXPECT_EQ(ps.group_id, "grp-1");
    EXPECT_EQ(ps.window.id, "w-1");
    ASSERT_EQ(ps.stages.size(), 1u);
    EXPECT_EQ(ps.stages[0].id, "stage-1");
}

// ---- Factories: invalid input throws std::invalid_argument -----------------

TEST(DomainFactories, MakeEndpointThrowsOnEmptyId) {
    EXPECT_THROW(make_endpoint("", "grp", "h"), std::invalid_argument);
}

TEST(DomainFactories, MakeEndpointThrowsOnEmptyGroupId) {
    EXPECT_THROW(make_endpoint("ep", "", "h"), std::invalid_argument);
}

TEST(DomainFactories, MakeWindowThrowsOnEmptyId) {
    EXPECT_THROW(make_window("", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z"),
                 std::invalid_argument);
}

TEST(DomainFactories, MakeWindowThrowsOnStartGreaterOrEqualEnd) {
    // start == end
    EXPECT_THROW(make_window("w", "2025-01-01T02:00:00Z", "2025-01-01T02:00:00Z"),
                 std::invalid_argument);
    // start > end
    EXPECT_THROW(make_window("w", "2025-01-01T04:00:00Z", "2025-01-01T02:00:00Z"),
                 std::invalid_argument);
}

TEST(DomainFactories, MakeScheduleThrowsOnEmptyId) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    RolloutStage rs{"stage-1", 1, {"grp-1"}};
    EXPECT_THROW(make_schedule("", "pkg", "grp", w, {rs}), std::invalid_argument);
}

TEST(DomainFactories, MakeScheduleThrowsOnEmptyPackage) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    RolloutStage rs{"stage-1", 1, {"grp-1"}};
    EXPECT_THROW(make_schedule("s", "", "grp", w, {rs}), std::invalid_argument);
}

TEST(DomainFactories, MakeScheduleThrowsOnEmptyGroupId) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    RolloutStage rs{"stage-1", 1, {"grp-1"}};
    EXPECT_THROW(make_schedule("s", "pkg", "", w, {rs}), std::invalid_argument);
}

TEST(DomainFactories, MakeScheduleThrowsOnNoStages) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    EXPECT_THROW(make_schedule("s", "pkg", "grp", w, {}), std::invalid_argument);
}

TEST(DomainFactories, MakeScheduleThrowsOnInvalidWindow) {
    MaintenanceWindow bad_w{"w-2", "2025-01-01T04:00:00Z", "2025-01-01T02:00:00Z"};
    RolloutStage rs{"stage-1", 1, {"grp-1"}};
    EXPECT_THROW(make_schedule("s", "pkg", "grp", bad_w, {rs}), std::invalid_argument);
}

// ---- Validation: is_valid() true for valid, false for invalid, never throws

TEST(DomainValidation, ValidEndpointIsValid) {
    Endpoint e = make_endpoint("ep-1", "grp-1", "host-1");
    EXPECT_TRUE(is_valid(e));
}

TEST(DomainValidation, InvalidEndpointIsInvalid) {
    Endpoint e{"", "grp", "h"};  // aggregate init bypasses factory
    EXPECT_FALSE(is_valid(e));
    Endpoint e2{"ep", "", "h"};
    EXPECT_FALSE(is_valid(e2));
}

TEST(DomainValidation, ValidWindowIsValid) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    EXPECT_TRUE(is_valid(w));
}

TEST(DomainValidation, InvalidWindowIsInvalid) {
    MaintenanceWindow empty_id{"", "s", "e"};
    EXPECT_FALSE(is_valid(empty_id));
    MaintenanceWindow empty_start{"w", "", "e"};
    EXPECT_FALSE(is_valid(empty_start));
    MaintenanceWindow empty_end{"w", "s", ""};
    EXPECT_FALSE(is_valid(empty_end));
    MaintenanceWindow start_ge_end{"w", "2025-01-01T04:00:00Z", "2025-01-01T02:00:00Z"};
    EXPECT_FALSE(is_valid(start_ge_end));
    MaintenanceWindow start_eq_end{"w", "2025-01-01T02:00:00Z", "2025-01-01T02:00:00Z"};
    EXPECT_FALSE(is_valid(start_eq_end));
}

TEST(DomainValidation, ValidStageIsValid) {
    RolloutStage rs{"stage-1", 1, {"grp-1"}};
    EXPECT_TRUE(is_valid(rs));
}

TEST(DomainValidation, InvalidStageIsInvalid) {
    RolloutStage empty_id{"", 1, {"grp-1"}};
    EXPECT_FALSE(is_valid(empty_id));
    RolloutStage no_groups{"stage-2", 2, {}};
    EXPECT_FALSE(is_valid(no_groups));
}

TEST(DomainValidation, ValidScheduleIsValid) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    RolloutStage rs{"stage-1", 1, {"grp-1"}};
    PatchSchedule ps = make_schedule("sch-1", "pkg-v2", "grp-1", w, {rs});
    EXPECT_TRUE(is_valid(ps));
}

TEST(DomainValidation, InvalidScheduleIsInvalid) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    RolloutStage rs{"stage-1", 1, {"grp-1"}};

    PatchSchedule empty_id{"", "pkg", "grp", w, {rs}};
    EXPECT_FALSE(is_valid(empty_id));
    PatchSchedule empty_pkg{"s", "", "grp", w, {rs}};
    EXPECT_FALSE(is_valid(empty_pkg));
    PatchSchedule empty_group{"s", "pkg", "", w, {rs}};
    EXPECT_FALSE(is_valid(empty_group));
    PatchSchedule no_stages{"s", "pkg", "grp", w, {}};
    EXPECT_FALSE(is_valid(no_stages));
    MaintenanceWindow bad_w{"w-2", "2025-01-01T04:00:00Z", "2025-01-01T02:00:00Z"};
    PatchSchedule bad_window{"s", "pkg", "grp", bad_w, {rs}};
    EXPECT_FALSE(is_valid(bad_window));
}

TEST(DomainValidation, IsValidNeverThrows) {
    // Directly-constructed invalid objects must be rejected without throwing.
    MaintenanceWindow bad_w{"w-2", "2025-01-01T04:00:00Z", "2025-01-01T02:00:00Z"};
    RolloutStage bad_rs{"", 1, {}};
    PatchSchedule bad_s{"", "", "", bad_w, {bad_rs}};

    EXPECT_NO_THROW(is_valid(Endpoint{"", "", ""}));
    EXPECT_NO_THROW(is_valid(MaintenanceWindow{"", "", ""}));
    EXPECT_NO_THROW(is_valid(RolloutStage{"", 0, {}}));
    EXPECT_NO_THROW(is_valid(bad_s));
}

// ---- Domain types ----------------------------------------------------------

TEST(DomainTypes, PatchStateEnumValues) {
    EXPECT_EQ(static_cast<int>(PatchState::Pending), 0);
    EXPECT_EQ(static_cast<int>(PatchState::Running), 1);
    EXPECT_EQ(static_cast<int>(PatchState::Paused), 2);
    EXPECT_EQ(static_cast<int>(PatchState::Failed), 3);
    EXPECT_EQ(static_cast<int>(PatchState::RolledBack), 4);
}

TEST(DomainTypes, RolloutStageOrder) {
    RolloutStage first{"stage-1", 1, {"grp-1"}};
    RolloutStage second{"stage-2", 2, {"grp-2"}};
    EXPECT_EQ(first.order, 1);
    EXPECT_EQ(second.order, 2);
    EXPECT_LT(first.order, second.order);  // lower order runs first
}

TEST(DomainTypes, PatchScheduleStoresStages) {
    MaintenanceWindow w =
        make_window("w-1", "2025-01-01T02:00:00Z", "2025-01-01T04:00:00Z");
    RolloutStage s1{"stage-1", 1, {"grp-1"}};
    RolloutStage s2{"stage-2", 2, {"grp-2"}};
    PatchSchedule ps = make_schedule("sch-1", "pkg-v2", "grp-1", w, {s1, s2});

    ASSERT_EQ(ps.stages.size(), 2u);
    EXPECT_EQ(ps.stages[0].id, "stage-1");
    EXPECT_EQ(ps.stages[0].order, 1);
    EXPECT_EQ(ps.stages[1].id, "stage-2");
    EXPECT_EQ(ps.stages[1].order, 2);
}

}  // namespace
