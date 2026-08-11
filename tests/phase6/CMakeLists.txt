# Phase 6 — C++ core unit tests (GoogleTest).
#
# Builds the `poc_domain_tests` executable against `poc_domain` and GoogleTest,
# and registers it with CTest. This file is only included when BUILD_TESTING is
# ON (see the root CMakeLists.txt), so the phase-1 build is unaffected when
# testing is disabled.

# Fetch GoogleTest via FetchContent (falls back to a system-installed gtest if
# the caller has already provided one via CMAKE_PREFIX_PATH / find_package).
include(FetchContent)

find_package(GTest QUIET)
if(NOT GTest_FOUND)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # googletest's own CMake sets BUILD_TESTING; keep it OFF to avoid
    # re-registering its internal tests.
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(poc_domain_tests domain_tests.cpp)
target_link_libraries(poc_domain_tests PRIVATE poc_domain GTest::gtest_main)
target_compile_features(poc_domain_tests PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(poc_domain_tests PRIVATE /W4)
else()
    target_compile_options(poc_domain_tests PRIVATE -Wall -Wextra)
endif()

include(GoogleTest)
gtest_discover_tests(poc_domain_tests)

# --- Sprint 2 (A2) tab/dock manager tests (Qt Test) -------------------------
# Builds the demo hub window plus the three embedded widgets and runs the
# layout save/restore, embedding, and persistence tests offscreen.
find_package(Qt6 6.2 QUIET COMPONENTS Widgets Network Test)

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)

    add_executable(a2_layout_tests
        a2_layout_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(a2_layout_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(a2_layout_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(a2_layout_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(a2_layout_tests PRIVATE /W4)
    else()
        target_compile_options(a2_layout_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME a2_layout_roundtrip
             COMMAND a2_layout_tests t1_layoutRoundTrip)
    add_test(NAME a2_embedding
             COMMAND a2_layout_tests t2_widgetEmbedding)
    add_test(NAME a2_persistence
             COMMAND a2_layout_tests t3_persistenceAcrossRestarts)
    set_tests_properties(a2_layout_roundtrip a2_embedding a2_persistence
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 3 (A3) shared app state tests (Qt Test) ---------------------
    # Builds DemoAppContext plus the demo hub and the three embedded widgets
    # and runs the context set/get, signal-emission, and shared-context tests
    # offscreen.
    add_executable(a3_context_tests
        a3_context_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(a3_context_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(a3_context_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(a3_context_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(a3_context_tests PRIVATE /W4)
    else()
        target_compile_options(a3_context_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME a3_state_setget
             COMMAND a3_context_tests t1_stateSetGet)
    add_test(NAME a3_signals
             COMMAND a3_context_tests t2_signalEmission)
    add_test(NAME a3_shared_context
             COMMAND a3_context_tests t3_sharedContext)
    set_tests_properties(a3_state_setget a3_signals a3_shared_context
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 4 (A4) Demo Mode engine tests (Qt Test) ---------------------
    # Builds DemoModeController plus DemoAppContext and runs the step
    # sequencing, pause/stop, narration, and auto-advance tests offscreen.
    add_executable(a4_demo_mode_tests
        a4_demo_mode_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_mode_controller.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(a4_demo_mode_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(a4_demo_mode_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(a4_demo_mode_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(a4_demo_mode_tests PRIVATE /W4)
    else()
        target_compile_options(a4_demo_mode_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME a4_sequencing
             COMMAND a4_demo_mode_tests t1_stepSequencing)
    add_test(NAME a4_pause_stop
             COMMAND a4_demo_mode_tests t2_pauseAndStop)
    add_test(NAME a4_narration
             COMMAND a4_demo_mode_tests t3_narrationUpdates)
    add_test(NAME a4_auto_advance
             COMMAND a4_demo_mode_tests t4_autoAdvance)
    set_tests_properties(a4_sequencing a4_pause_stop a4_narration a4_auto_advance
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 5 (A5) demo script parser tests (Qt Test) -------------------
    # Builds the self-contained JSON demo script parser and runs the valid-parse,
    # step-type, invalid-reject, and schema-validation tests offscreen.
    add_executable(a5_script_parser_tests
        a5_script_parser_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_script_parser.cpp
    )
    target_include_directories(a5_script_parser_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(a5_script_parser_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(a5_script_parser_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(a5_script_parser_tests PRIVATE /W4)
    else()
        target_compile_options(a5_script_parser_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME a5_valid_parse
             COMMAND a5_script_parser_tests t1_validParse)
    add_test(NAME a5_step_types
             COMMAND a5_script_parser_tests t2_stepTypes)
    add_test(NAME a5_invalid_reject
             COMMAND a5_script_parser_tests t3_invalidReject)
    add_test(NAME a5_schema
             COMMAND a5_script_parser_tests t4_schemaValidation)
    set_tests_properties(a5_valid_parse a5_step_types a5_invalid_reject a5_schema
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 6 (A6) Demo Mode UI tests (Qt Test) -------------------------
    # Builds DemoModeBar plus the controller and the S5 parser and runs the
    # button-action, step-indicator, narration, and end-to-end integration
    # tests offscreen.
    add_executable(a6_demo_mode_bar_tests
        a6_demo_mode_bar_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_mode_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_mode_controller.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_script_parser.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(a6_demo_mode_bar_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(a6_demo_mode_bar_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(a6_demo_mode_bar_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(a6_demo_mode_bar_tests PRIVATE /W4)
    else()
        target_compile_options(a6_demo_mode_bar_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME a6_buttons
             COMMAND a6_demo_mode_bar_tests t1_buttonActions)
    add_test(NAME a6_indicator
             COMMAND a6_demo_mode_bar_tests t2_stepIndicator)
    add_test(NAME a6_narration
             COMMAND a6_demo_mode_bar_tests t3_narration)
    add_test(NAME a6_integration
             COMMAND a6_demo_mode_bar_tests t4_integration)
    set_tests_properties(a6_buttons a6_indicator a6_narration a6_integration
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 7 (A7) Roadmap/Future tab tests (Qt Test) ------------------
    # Builds RoadmapTab and runs the render, scroll, style, and empty-model
    # tests offscreen.
    add_executable(a7_roadmap_tab_tests
        a7_roadmap_tab_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
    )
    target_include_directories(a7_roadmap_tab_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(a7_roadmap_tab_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(a7_roadmap_tab_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(a7_roadmap_tab_tests PRIVATE /W4)
    else()
        target_compile_options(a7_roadmap_tab_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME a7_render
             COMMAND a7_roadmap_tab_tests t1_rendersItems)
    add_test(NAME a7_scroll
             COMMAND a7_roadmap_tab_tests t2_scrollable)
    add_test(NAME a7_style
             COMMAND a7_roadmap_tab_tests t3_styled)
    add_test(NAME a7_empty
             COMMAND a7_roadmap_tab_tests t4_emptyModel)
    set_tests_properties(a7_render a7_scroll a7_style a7_empty
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 8 (A8) roadmap content model tests (Qt Test) --------------
    # Builds the A8 content model plus RoadmapTab and runs the parse/load,
    # field-access, render-all-items, and invalid-source tests offscreen.
    add_executable(a8_roadmap_model_tests
        a8_roadmap_model_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
    )
    target_include_directories(a8_roadmap_model_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(a8_roadmap_model_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(a8_roadmap_model_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(a8_roadmap_model_tests PRIVATE /W4)
    else()
        target_compile_options(a8_roadmap_model_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME a8_parse
             COMMAND a8_roadmap_model_tests t1_parsesLoads)
    add_test(NAME a8_fields
             COMMAND a8_roadmap_model_tests t2_fieldAccess)
    add_test(NAME a8_render
             COMMAND a8_roadmap_model_tests t3_rendersAllItems)
    add_test(NAME a8_invalid
             COMMAND a8_roadmap_model_tests t4_invalidRejected)
    set_tests_properties(a8_parse a8_fields a8_render a8_invalid
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 16 (B6) dashboard live-reaction tests (Qt Test) ------------
    # Builds the dashboard and runs the event-triggered re-render, no-poll,
    # multiple-events, and regression tests offscreen.
    add_executable(b6_dashboard_live_tests
        b6_dashboard_live_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
    )
    target_include_directories(b6_dashboard_live_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(b6_dashboard_live_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(b6_dashboard_live_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(b6_dashboard_live_tests PRIVATE /W4)
    else()
        target_compile_options(b6_dashboard_live_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME b6_event_rerender
             COMMAND b6_dashboard_live_tests t1_eventTriggersRerender)
    add_test(NAME b6_no_poll
             COMMAND b6_dashboard_live_tests t2_noRelianceOnPollTimer)
    add_test(NAME b6_multiple_events
             COMMAND b6_dashboard_live_tests t3_multipleEvents)
    add_test(NAME b6_regression
             COMMAND b6_dashboard_live_tests t4_regression)
    set_tests_properties(b6_event_rerender b6_no_poll b6_multiple_events b6_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 17 (B7) control-action feedback tests (Qt Test) ------------
    # Builds the control panel and runs the state-diff, confirmation, per-action,
    # and regression tests offscreen.
    add_executable(b7_control_feedback_tests
        b7_control_feedback_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(b7_control_feedback_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(b7_control_feedback_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(b7_control_feedback_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(b7_control_feedback_tests PRIVATE /W4)
    else()
        target_compile_options(b7_control_feedback_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME b7_state_diff
             COMMAND b7_control_feedback_tests t1_stateDiffDisplayed)
    add_test(NAME b7_confirmation
             COMMAND b7_control_feedback_tests t2_confirmationVisible)
    add_test(NAME b7_per_action
             COMMAND b7_control_feedback_tests t3_perActionDiff)
    add_test(NAME b7_regression
             COMMAND b7_control_feedback_tests t4_regression)
    set_tests_properties(b7_state_diff b7_confirmation b7_per_action b7_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 18 (C1) animated progress bar tests (Qt Test) ------------
    # Builds the animated progress bar plus the dashboard and runs the
    # animation-target, per-endpoint, target-change, and regression tests
    # offscreen.
    add_executable(c1_animated_progress_tests
        c1_animated_progress_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
    )
    target_include_directories(c1_animated_progress_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(c1_animated_progress_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(c1_animated_progress_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(c1_animated_progress_tests PRIVATE /W4)
    else()
        target_compile_options(c1_animated_progress_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME c1_animation_target
             COMMAND c1_animated_progress_tests t1_animationReachesTarget)
    add_test(NAME c1_per_endpoint
             COMMAND c1_animated_progress_tests t2_perEndpointBars)
    add_test(NAME c1_target_change
             COMMAND c1_animated_progress_tests t3_targetChangeReanimates)
    add_test(NAME c1_regression
             COMMAND c1_animated_progress_tests t4_regression)
    set_tests_properties(c1_animation_target c1_per_endpoint c1_target_change c1_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 19 (C2) color-coded state tests (Qt Test) ------------
    # Builds the dashboard and runs the state->color mapping, applied-
    # consistently, and regression tests offscreen.
    add_executable(c2_color_state_tests
        c2_color_state_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
    )
    target_include_directories(c2_color_state_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(c2_color_state_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(c2_color_state_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(c2_color_state_tests PRIVATE /W4)
    else()
        target_compile_options(c2_color_state_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME c2_mapping
             COMMAND c2_color_state_tests t1_stateColorMapping)
    add_test(NAME c2_consistency
             COMMAND c2_color_state_tests t2_appliedConsistently)
    add_test(NAME c2_regression
             COMMAND c2_color_state_tests t3_regression)
    set_tests_properties(c2_mapping c2_consistency c2_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 20 (C3) state badge renderer tests (Qt Test) ------------
    # Builds the reusable StateBadge plus the dashboard and runs the
    # badge-render, reusable, legend, and regression tests offscreen.
    add_executable(c3_state_badge_tests
        c3_state_badge_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
    )
    target_include_directories(c3_state_badge_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(c3_state_badge_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(c3_state_badge_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(c3_state_badge_tests PRIVATE /W4)
    else()
        target_compile_options(c3_state_badge_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME c3_badge_render
             COMMAND c3_state_badge_tests t1_badgeRender)
    add_test(NAME c3_reusable
             COMMAND c3_state_badge_tests t2_reusable)
    add_test(NAME c3_legend
             COMMAND c3_state_badge_tests t3_legend)
    add_test(NAME c3_regression
             COMMAND c3_state_badge_tests t4_regression)
    set_tests_properties(c3_badge_render c3_reusable c3_legend c3_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 21 (C4) fleet summary panel tests (Qt Test) ------------
    # Builds the fleet summary panel plus the dashboard and runs the
    # count-aggregation, total, updates-on-change, and regression tests
    # offscreen.
    add_executable(c4_fleet_summary_tests
        c4_fleet_summary_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
    )
    target_include_directories(c4_fleet_summary_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(c4_fleet_summary_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(c4_fleet_summary_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(c4_fleet_summary_tests PRIVATE /W4)
    else()
        target_compile_options(c4_fleet_summary_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME c4_aggregation
             COMMAND c4_fleet_summary_tests t1_aggregation)
    add_test(NAME c4_total
             COMMAND c4_fleet_summary_tests t2_total)
    add_test(NAME c4_updates
             COMMAND c4_fleet_summary_tests t3_updates)
    add_test(NAME c4_regression
             COMMAND c4_fleet_summary_tests t4_regression)
    set_tests_properties(c4_aggregation c4_total c4_updates c4_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 22 (C5) rollout-stage grouping tests (Qt Test) ------------
    # Builds the dashboard and runs the grouping-logic, stage-header,
    # per-stage-progress, and regression tests offscreen.
    add_executable(c5_rollout_grouping_tests
        c5_rollout_grouping_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
    )
    target_include_directories(c5_rollout_grouping_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(c5_rollout_grouping_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(c5_rollout_grouping_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(c5_rollout_grouping_tests PRIVATE /W4)
    else()
        target_compile_options(c5_rollout_grouping_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME c5_grouping
             COMMAND c5_rollout_grouping_tests t1_grouping)
    add_test(NAME c5_headers
             COMMAND c5_rollout_grouping_tests t2_headers)
    add_test(NAME c5_progress
             COMMAND c5_rollout_grouping_tests t3_progress)
    add_test(NAME c5_regression
             COMMAND c5_rollout_grouping_tests t4_regression)
    set_tests_properties(c5_grouping c5_headers c5_progress c5_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 23 (C6) smooth refresh tests (Qt Test) ------------
    # Builds the dashboard and runs the smooth-update, no-flicker,
    # stream-driven, and regression tests offscreen.
    add_executable(c6_smooth_refresh_tests
        c6_smooth_refresh_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
    )
    target_include_directories(c6_smooth_refresh_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(c6_smooth_refresh_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(c6_smooth_refresh_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(c6_smooth_refresh_tests PRIVATE /W4)
    else()
        target_compile_options(c6_smooth_refresh_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME c6_smooth
             COMMAND c6_smooth_refresh_tests t1_smooth)
    add_test(NAME c6_no_flicker
             COMMAND c6_smooth_refresh_tests t2_noFlicker)
    add_test(NAME c6_stream_driven
             COMMAND c6_smooth_refresh_tests t3_streamDriven)
    add_test(NAME c6_regression
             COMMAND c6_smooth_refresh_tests t4_regression)
    set_tests_properties(c6_smooth c6_no_flicker c6_stream_driven c6_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 24 (C7) dashboard legend tests (Qt Test) ------------
    # Builds the dashboard legend plus the dashboard and runs the
    # legend-renders, all-states, and regression tests offscreen.
    add_executable(c7_dashboard_legend_tests
        c7_dashboard_legend_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
    )
    target_include_directories(c7_dashboard_legend_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(c7_dashboard_legend_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(c7_dashboard_legend_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(c7_dashboard_legend_tests PRIVATE /W4)
    else()
        target_compile_options(c7_dashboard_legend_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME c7_legend_renders
             COMMAND c7_dashboard_legend_tests t1_legendRenders)
    add_test(NAME c7_all_states
             COMMAND c7_dashboard_legend_tests t2_allStates)
    add_test(NAME c7_regression
             COMMAND c7_dashboard_legend_tests t3_regression)
    set_tests_properties(c7_legend_renders c7_all_states c7_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 25 (D1) fleet size config tests (Qt Test) ------------------
    # Builds the fleet-size spin box control plus the shared context and runs
    # the spin-box range/set, control-updates-context, and regression tests
    # offscreen.
    add_executable(d1_spinbox_tests
        d1_spinbox_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(d1_spinbox_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d1_spinbox_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d1_spinbox_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d1_spinbox_tests PRIVATE /W4)
    else()
        target_compile_options(d1_spinbox_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d1_spinbox
             COMMAND d1_spinbox_tests)
    set_tests_properties(d1_spinbox
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d1_control_context_tests
        d1_control_context_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(d1_control_context_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d1_control_context_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d1_control_context_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d1_control_context_tests PRIVATE /W4)
    else()
        target_compile_options(d1_control_context_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d1_control_context
             COMMAND d1_control_context_tests)
    set_tests_properties(d1_control_context
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d1_regression_tests
        d1_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d1_regression_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d1_regression_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d1_regression_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d1_regression_tests PRIVATE /W4)
    else()
        target_compile_options(d1_regression_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d1_regression
             COMMAND d1_regression_tests)
    set_tests_properties(d1_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 26 (D2) failure rate config tests (Qt Test) ---------------
    # Builds the failure-rate control plus the shared context and runs the
    # range/set, control-updates-context, and regression tests offscreen.
    add_executable(d2_range_tests
        d2_range_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(d2_range_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d2_range_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d2_range_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d2_range_tests PRIVATE /W4)
    else()
        target_compile_options(d2_range_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d2_range
             COMMAND d2_range_tests)
    set_tests_properties(d2_range
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d2_control_context_tests
        d2_control_context_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(d2_control_context_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d2_control_context_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d2_control_context_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d2_control_context_tests PRIVATE /W4)
    else()
        target_compile_options(d2_control_context_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d2_control_context
             COMMAND d2_control_context_tests)
    set_tests_properties(d2_control_context
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d2_regression_tests
        d2_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d2_regression_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d2_regression_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d2_regression_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d2_regression_tests PRIVATE /W4)
    else()
        target_compile_options(d2_regression_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d2_regression
             COMMAND d2_regression_tests)
    set_tests_properties(d2_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 27 (D3) seed config tests (Qt Test) -----------------------
    # Builds the seed control plus the shared context and runs the
    # accepts-integer-seed, control-updates-context, and regression tests
    # offscreen.
    add_executable(d3_seed_tests
        d3_seed_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(d3_seed_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d3_seed_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d3_seed_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d3_seed_tests PRIVATE /W4)
    else()
        target_compile_options(d3_seed_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d3_seed
             COMMAND d3_seed_tests)
    set_tests_properties(d3_seed
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d3_control_context_tests
        d3_control_context_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(d3_control_context_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d3_control_context_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d3_control_context_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d3_control_context_tests PRIVATE /W4)
    else()
        target_compile_options(d3_control_context_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d3_control_context
             COMMAND d3_control_context_tests)
    set_tests_properties(d3_control_context
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d3_regression_tests
        d3_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
    )
    target_include_directories(d3_regression_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d3_regression_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d3_regression_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d3_regression_tests PRIVATE /W4)
    else()
        target_compile_options(d3_regression_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d3_regression
             COMMAND d3_regression_tests)
    set_tests_properties(d3_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 28 (D4) scenario preset tests (Qt Test) ------------------
    # Builds the D4 scenario model/presets plus the shared context and runs
    # the preset-data-correctness, loads-correct-config-values, and
    # regression tests offscreen.
    add_executable(d4_presets_tests
        d4_presets_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(d4_presets_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d4_presets_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d4_presets_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d4_presets_tests PRIVATE /W4)
    else()
        target_compile_options(d4_presets_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d4_presets
             COMMAND d4_presets_tests)
    set_tests_properties(d4_presets
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d4_load_tests
        d4_load_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(d4_load_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d4_load_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d4_load_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d4_load_tests PRIVATE /W4)
    else()
        target_compile_options(d4_load_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d4_load
             COMMAND d4_load_tests)
    set_tests_properties(d4_load
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d4_regression_tests
        d4_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d4_regression_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d4_regression_tests PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d4_regression_tests PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d4_regression_tests PRIVATE /W4)
    else()
        target_compile_options(d4_regression_tests PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d4_regression
             COMMAND d4_regression_tests)
    set_tests_properties(d4_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 29 (D5) scenario selector tests (Qt Test) ------------------
    # Builds the scenario selector wired to the config controls and the shared
    # context and runs the preset-load, override, and regression tests
    # offscreen.
    add_executable(d5_preset_load
        d5_preset_load_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d5_preset_load PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d5_preset_load PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d5_preset_load PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d5_preset_load PRIVATE /W4)
    else()
        target_compile_options(d5_preset_load PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d5_preset_load
             COMMAND d5_preset_load)
    set_tests_properties(d5_preset_load
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d5_override
        d5_override_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d5_override PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d5_override PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d5_override PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d5_override PRIVATE /W4)
    else()
        target_compile_options(d5_override PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d5_override
             COMMAND d5_override)
    set_tests_properties(d5_override
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d5_regression
        d5_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d5_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d5_regression PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d5_regression PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d5_regression PRIVATE /W4)
    else()
        target_compile_options(d5_regression PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d5_regression
             COMMAND d5_regression)
    set_tests_properties(d5_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 30 (D6) config validation tests (Qt Test) ----------------
    # Builds the ConfigValidator plus the control panel and the shared context
    # and runs the rules, blocked-action, inline-error, and regression tests
    # offscreen.
    add_executable(d6_rules
        d6_rules_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
    )
    target_include_directories(d6_rules PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d6_rules PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d6_rules PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d6_rules PRIVATE /W4)
    else()
        target_compile_options(d6_rules PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d6_rules
             COMMAND d6_rules)
    set_tests_properties(d6_rules
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d6_blocked
        d6_blocked_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d6_blocked PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d6_blocked PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d6_blocked PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d6_blocked PRIVATE /W4)
    else()
        target_compile_options(d6_blocked PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d6_blocked
             COMMAND d6_blocked)
    set_tests_properties(d6_blocked
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d6_errors
        d6_errors_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d6_errors PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d6_errors PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d6_errors PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d6_errors PRIVATE /W4)
    else()
        target_compile_options(d6_errors PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d6_errors
             COMMAND d6_errors)
    set_tests_properties(d6_errors
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(d6_regression
        d6_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(d6_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(d6_regression PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(d6_regression PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(d6_regression PRIVATE /W4)
    else()
        target_compile_options(d6_regression PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME d6_regression
             COMMAND d6_regression)
    set_tests_properties(d6_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 35 (E4) audit log panel tests (Qt Test) ------------------
    # Builds the AuditLogPanel and runs the render, update, and regression
    # tests offscreen.
    add_executable(e4_render
        e4_render_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(e4_render PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e4_render PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e4_render PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e4_render PRIVATE /W4)
    else()
        target_compile_options(e4_render PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e4_render
             COMMAND e4_render)
    set_tests_properties(e4_render
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(e4_updates
        e4_updates_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(e4_updates PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e4_updates PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e4_updates PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e4_updates PRIVATE /W4)
    else()
        target_compile_options(e4_updates PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e4_updates
             COMMAND e4_updates)
    set_tests_properties(e4_updates
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(e4_regression
        e4_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(e4_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e4_regression PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e4_regression PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e4_regression PRIVATE /W4)
    else()
        target_compile_options(e4_regression PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e4_regression
             COMMAND e4_regression)
    set_tests_properties(e4_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 36 (E5) timestamp formatting tests (Qt Test) --------------
    # Builds the self-contained timestamp formatting helper and runs the
    # format, readable, local-time-correct, offset, and invalid-input tests
    # offscreen.
    add_executable(e5_formatting
        e5_formatting_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
    )
    target_include_directories(e5_formatting PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e5_formatting PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e5_formatting PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e5_formatting PRIVATE /W4)
    else()
        target_compile_options(e5_formatting PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e5_formatting
             COMMAND e5_formatting)
    set_tests_properties(e5_formatting
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(e5_display
        e5_display_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(e5_display PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e5_display PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e5_display PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e5_display PRIVATE /W4)
    else()
        target_compile_options(e5_display PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e5_display
             COMMAND e5_display)
    set_tests_properties(e5_display
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(e5_regression
        e5_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(e5_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e5_regression PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e5_regression PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e5_regression PRIVATE /W4)
    else()
        target_compile_options(e5_regression PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e5_regression
             COMMAND e5_regression)
    set_tests_properties(e5_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 37 (E6) log auto-refresh tests (Qt Test) ------------------
    # Builds the AuditLogPanel plus the shared context and runs the refresh-on-
    # event, multiple-event, and regression tests offscreen. The refresh logic
    # is driven deterministically via applyFetchedLog (no live server needed).
    add_executable(e6_refresh
        e6_refresh_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(e6_refresh PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e6_refresh PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e6_refresh PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e6_refresh PRIVATE /W4)
    else()
        target_compile_options(e6_refresh PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e6_refresh
             COMMAND e6_refresh)
    set_tests_properties(e6_refresh
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(e6_multiple
        e6_multiple_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(e6_multiple PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e6_multiple PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e6_multiple PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e6_multiple PRIVATE /W4)
    else()
        target_compile_options(e6_multiple PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e6_multiple
             COMMAND e6_multiple)
    set_tests_properties(e6_multiple
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(e6_regression
        e6_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(e6_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e6_regression PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e6_regression PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e6_regression PRIVATE /W4)
    else()
        target_compile_options(e6_regression PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e6_regression
             COMMAND e6_regression)
    set_tests_properties(e6_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    # --- Sprint 38 (E7) log export tests (Qt Test) -------------------------
    # Builds the AuditLogPanel plus the shared context and runs the export
    # format/content, valid-CSV, and regression tests offscreen. Exports are
    # written to a temp path and read back to assert validity.
    add_executable(e7_export
        e7_export_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(e7_export PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e7_export PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e7_export PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e7_export PRIVATE /W4)
    else()
        target_compile_options(e7_export PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e7_export
             COMMAND e7_export)
    set_tests_properties(e7_export
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(e7_valid
        e7_valid_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
    )
    target_include_directories(e7_valid PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e7_valid PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e7_valid PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e7_valid PRIVATE /W4)
    else()
        target_compile_options(e7_valid PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e7_valid
             COMMAND e7_valid)
    set_tests_properties(e7_valid
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(e7_regression
        e7_regression_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/audit_log_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/timestamp_format.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_main_window.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_presets.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/scenario_selector.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/config_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_size_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/failure_rate_control.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/seed_control.cpp
    )
    target_include_directories(e7_regression PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(e7_regression PRIVATE Qt6::Widgets Qt6::Network Qt6::Test)
    target_compile_features(e7_regression PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(e7_regression PRIVATE /W4)
    else()
        target_compile_options(e7_regression PRIVATE -Wall -Wextra)
    endif()

    add_test(NAME e7_regression
             COMMAND e7_regression)
    set_tests_properties(e7_regression
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
