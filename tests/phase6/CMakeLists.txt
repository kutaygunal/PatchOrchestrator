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
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
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
        ${CMAKE_SOURCE_DIR}/src/ui/schedule_editor.cpp
        ${CMAKE_SOURCE_DIR}/src/ui/control_panel.cpp
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
        ${CMAKE_SOURCE_DIR}/src/ui/demo_app_context.cpp
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
endif()
