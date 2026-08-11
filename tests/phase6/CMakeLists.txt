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
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
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
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_tab.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/roadmap_model.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/animated_progress_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/state_badge.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/fleet_summary_panel.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/dashboard_legend.cpp
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
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
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
endif()
