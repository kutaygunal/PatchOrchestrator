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
