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
