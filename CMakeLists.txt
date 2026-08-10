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

# --- Packaging / installer (Phase 14) -----------------------------------------
# Bundle the Qt runtime DLLs next to the installed executables with
# windeployqt (see cmake/bundle_qt.cmake) BEFORE cpack packages the tree.
install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/bundle_qt.cmake")

# CPack configuration for a redistributable installer of the Qt apps.
# NOTE: set the CPACK_* variables BEFORE include(CPack) so they are captured
# into the generated CPackConfig.cmake.
set(CPACK_PACKAGE_NAME "PatchOrchestrator")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_VENDOR "PatchOrchestrator")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Qt control plane for scheduling, pausing, and rolling back fleet-wide software patches")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
# Redistributable ZIP (runs anywhere after unzip; the Qt runtime is bundled by
# the windeployqt install step above).
set(CPACK_GENERATOR "ZIP")
set(CPACK_STRIP_FILES TRUE)

include(CPack)
