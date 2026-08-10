#!/usr/bin/env bash
#
# Phase 6 verification runner (scrum-master authored).
# Configures the CMake build with testing enabled, builds the test target, and
# runs the C++ unit tests via CTest.
#
# Compiler-agnostic: works with MSVC `cl` (inside the VS 2022 developer
# environment) or a g++-style compiler.
#
# On this machine there is NO g++/clang on PATH. MSVC `cl` is only available
# inside the VS 2022 developer environment. Run this runner inside it:
#
#   cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash tests/phase6/run_ctest.sh"
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 300 bash tests/phase6/run_ctest.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build.p6"
LOG_DIR="${BUILD_DIR}"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }

# --- 0. Preflight ---
if ! command -v cmake >/dev/null 2>&1; then
  echo "ERROR: cmake not found on PATH."
  exit 2
fi
echo "cmake: $(cmake --version | head -n1)"

# Ensure the build/log directory exists before redirecting logs into it.
mkdir -p "${BUILD_DIR}"

# --- 1. Configure with testing enabled ---
echo "== [1] cmake configure (BUILD_TESTING=ON) =="
if cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DBUILD_TESTING=ON \
     >"${LOG_DIR}/configure.log" 2>&1; then
  pass "cmake configure succeeded"
else
  fail "cmake configure failed (see ${LOG_DIR}/configure.log)"
fi

# --- 2. Build all targets (includes the gtest test executable) ---
echo "== [2] cmake build =="
if cmake --build "${BUILD_DIR}" --config Release >"${LOG_DIR}/build.log" 2>&1; then
  pass "cmake build succeeded"
else
  fail "cmake build failed (see ${LOG_DIR}/build.log)"
fi

# --- 3. Run CTest ---
echo "== [3] ctest =="
if ctest --test-dir "${BUILD_DIR}" -C Release --output-on-failure \
     >"${LOG_DIR}/ctest.log" 2>&1; then
  pass "ctest passed"
else
  fail "ctest failed (see ${LOG_DIR}/ctest.log)"
fi
cat "${LOG_DIR}/ctest.log"

# --- Summary ---
echo ""
if [ "${FAILURES}" -eq 0 ]; then
  echo "RESULT: PASS"
  exit 0
else
  echo "RESULT: FAIL (${FAILURES} check(s) failed)"
  exit 1
fi
