# PatchOrchestrator — root CMake build.
#
# Wires the C++/Qt target (src/) and stubs the Python simulation engine
# (python/) and .NET REST API (dotnet/) sub-builds so they can be added in
# later phases without restructuring the build.

cmake_minimum_required(VERSION 3.16)

project(PatchOrchestrator
    VERSION 0.1.0
    DESCRIPTION "Qt control plane for scheduling, pausing, and rolling back fleet-wide software patches"
    LANGUAGES CXX)

# --- Global options ----------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# --- Testing (Phase 6) -------------------------------------------------------
# Standard CMake option. When ON, the C++ core unit tests (GoogleTest) are
# built and registered with CTest. When OFF, the phase-1 build is unaffected.
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests/phase6)
endif()

# --- Qt discovery hint -------------------------------------------------------
# Make Qt6 discoverable for a plain `cmake -S . -B build` invocation (as used
# by the Phase 1 verification script and CI) without requiring the caller to
# set Qt6_DIR. Only used if the caller has not already provided a prefix path.
if(NOT CMAKE_PREFIX_PATH)
    foreach(_qt_root
        "C:/Qt/6.8.2/msvc2022_64"
        "$ENV{QT_ROOT}"
        "$ENV{QT_DIR}")
        if(EXISTS "${_qt_root}")
            list(APPEND CMAKE_PREFIX_PATH "${_qt_root}")
        endif()
    endforeach()
endif()

# --- C++/Qt target -----------------------------------------------------------
add_subdirectory(src)

# --- Python simulation engine (stub, added in Phase 3) -----------------------
# The Python engine is a pure-Python package; CMake only registers a
# placeholder custom target so the build graph is stable. Phase 3 replaces
# this with real packaging/test wiring.
add_subdirectory(python)

# --- .NET REST API (stub, added in Phase 5) -----------------------------------
# The .NET API is built with `dotnet build`; CMake only registers a
# placeholder custom target. Phase 5 replaces this with a real build step.
add_subdirectory(dotnet)
