# PatchOrchestrator — C++/Qt target (Phase 1 placeholder).
#
# This target is the C++/Qt control-plane application. In Phase 1 it is a
# minimal Qt Core console app that produces a verifiable artifact. Later
# phases add the Qt Widgets dashboard, the domain model, and unit tests here.

# Locate Qt 6. Prefer the Qt6_DIR cache variable; fall back to a standard
# install location so the build works both locally and in CI.
find_package(Qt6 6.2 QUIET COMPONENTS Core)

if(NOT Qt6_FOUND)
    message(WARNING "Qt6 not found — building a plain C++ placeholder instead of the Qt target.")
    add_executable(patchorchestrator main.cpp)
    target_compile_features(patchorchestrator PRIVATE cxx_std_17)
    target_compile_definitions(patchorchestrator PRIVATE PATCHORCHESTRATOR_VERSION="0.1.0")
else()
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    add_executable(patchorchestrator main.cpp)
    target_link_libraries(patchorchestrator PRIVATE Qt6::Core)
    target_compile_features(patchorchestrator PRIVATE cxx_std_17)
    target_compile_definitions(patchorchestrator PRIVATE PATCHORCHESTRATOR_VERSION="0.1.0")
endif()

# Install the executable so the build produces a well-known artifact path.
install(TARGETS patchorchestrator RUNTIME DESTINATION bin)

# --- Domain model library (Phase 2) -------------------------------------------
# Plain C++ domain model for the patch control plane. No GUI, no I/O, no Qt
# dependencies. Compiled with warnings-as-errors.
add_library(poc_domain STATIC domain/domain.cpp)
target_include_directories(poc_domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(poc_domain PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(poc_domain PRIVATE /W4 /WX)
else()
    target_compile_options(poc_domain PRIVATE -Wall -Wextra -Werror)
endif()

# --- Qt dashboard UI (Phase 8) ------------------------------------------------
# Read-only dashboard that lists simulated endpoints and their patch status,
# polling/refreshing against the .NET API boundary (Phase 7).
find_package(Qt6 6.2 QUIET COMPONENTS Widgets Network)

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)

    add_executable(patchorchestrator_ui
        ui/dashboard_main.cpp
        ui/dashboard.cpp
        ui/animated_progress_bar.cpp
        ui/demo_app_context.cpp
        ui/state_badge.cpp
        ui/fleet_summary_panel.cpp
    )
    target_link_libraries(patchorchestrator_ui PRIVATE Qt6::Widgets Qt6::Network)
    target_compile_features(patchorchestrator_ui PRIVATE cxx_std_17)
    target_compile_definitions(patchorchestrator_ui PRIVATE PATCHORCHESTRATOR_VERSION="0.1.0")

    install(TARGETS patchorchestrator_ui RUNTIME DESTINATION bin)
else()
    message(WARNING "Qt6 Widgets/Network not found — skipping patchorchestrator_ui target.")
endif()

# --- Schedule-definition UI (Phase 9) ----------------------------------------
# Forms for group, maintenance-window, and rollout-stage editors wired to the
# API to create a schedule. Consumes the working .NET API boundary (Phase 7).
find_package(Qt6 6.2 QUIET COMPONENTS Widgets Network)

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)

    add_executable(patchorchestrator_schedule_ui
        ui/schedule_main.cpp
        ui/schedule_editor.cpp
        ui/demo_app_context.cpp
    )
    target_link_libraries(patchorchestrator_schedule_ui PRIVATE Qt6::Widgets Qt6::Network)
    target_compile_features(patchorchestrator_schedule_ui PRIVATE cxx_std_17)
    target_compile_definitions(patchorchestrator_schedule_ui PRIVATE PATCHORCHESTRATOR_VERSION="0.1.0")

    install(TARGETS patchorchestrator_schedule_ui RUNTIME DESTINATION bin)
else()
    message(WARNING "Qt6 Widgets/Network not found — skipping patchorchestrator_schedule_ui target.")
endif()

# --- Control-actions UI (Phase 10) -------------------------------------------
# Schedule / Pause / Resume / Rollback buttons wired to the API endpoints,
# with confirmation dialogs and result feedback. Consumes the working .NET API
# boundary (Phase 7).
find_package(Qt6 6.2 QUIET COMPONENTS Widgets Network)

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)

    add_executable(patchorchestrator_control_ui
        ui/control_main.cpp
        ui/control_panel.cpp
        ui/demo_app_context.cpp
    )
    target_link_libraries(patchorchestrator_control_ui PRIVATE Qt6::Widgets Qt6::Network)
    target_compile_features(patchorchestrator_control_ui PRIVATE cxx_std_17)
    target_compile_definitions(patchorchestrator_control_ui PRIVATE PATCHORCHESTRATOR_VERSION="0.1.0")

    install(TARGETS patchorchestrator_control_ui RUNTIME DESTINATION bin)
else()
    message(WARNING "Qt6 Widgets/Network not found — skipping patchorchestrator_control_ui target.")
endif()

# --- Unified demo hub (Sprint 1 / A1) --------------------------------------
# Hosts the dashboard, schedule editor, and control panel as tabs in a single
# application. Reuses the existing widget classes unchanged.
find_package(Qt6 6.2 QUIET COMPONENTS Widgets Network)

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)

    add_executable(patchorchestrator_demo
        ui/demo_main.cpp
        ui/demo_main_window.cpp
        ui/demo_app_context.cpp
        ui/demo_mode_controller.cpp
        ui/demo_mode_bar.cpp
        ui/demo_script_parser.cpp
        ui/roadmap_tab.cpp
        ui/roadmap_model.cpp
        ui/dashboard.cpp
        ui/animated_progress_bar.cpp
        ui/schedule_editor.cpp
        ui/control_panel.cpp
        ui/state_badge.cpp
        ui/fleet_summary_panel.cpp
    )
    target_link_libraries(patchorchestrator_demo PRIVATE Qt6::Widgets Qt6::Network)
    target_include_directories(patchorchestrator_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_features(patchorchestrator_demo PRIVATE cxx_std_17)
    target_compile_definitions(patchorchestrator_demo PRIVATE PATCHORCHESTRATOR_VERSION="0.1.0")

    install(TARGETS patchorchestrator_demo RUNTIME DESTINATION bin)

    # Copy the declarative roadmap source next to the demo executable so the
    # A8 content model loads the real JSON file at runtime (falling back to the
    # built-in default if the file is absent).
    add_custom_command(TARGET patchorchestrator_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/ui/roadmap.json
            $<TARGET_FILE_DIR:patchorchestrator_demo>/roadmap.json)
else()
    message(WARNING "Qt6 Widgets/Network not found — skipping patchorchestrator_demo target.")
endif()
