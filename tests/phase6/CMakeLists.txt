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
endif()
