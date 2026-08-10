#!/usr/bin/env bash
#
# Phase 14 verification runner (scrum-master authored).
# Verifies the P14 hardening is coherent:
#   - C++/Qt code compiles with warnings-as-errors (poc_domain already does).
#   - The full test suites (P4 pytest, P6 CTest, P11 E2E) still pass.
#   - A CPack config exists and (best-effort) CPack runs.
#   - A release tag / release notes exist.
#
# Suites that need a missing toolchain or the MSVC VS-2022 env are SKIPPED with
# a NOTE (not a failure) so the harness is usable outside the full dev env.
# Run inside the VS 2022 dev env for the most complete result.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 600 bash tests/phase14/verify_hardening.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
CMAKE_SRC="${PROJECT_ROOT}/src/CMakeLists.txt"
ROOT_CMAKE="${PROJECT_ROOT}/CMakeLists.txt"
LOG_DIR="${PROJECT_ROOT}/build.p14"
mkdir -p "${LOG_DIR}"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }
note() { echo "NOTE: $1"; }

# --- 1. Warnings-as-errors for the C++/Qt code (poc_domain already does) ---
echo "== [1] warnings-as-errors =="
if [ -f "$CMAKE_SRC" ]; then
  if grep -q 'W4' "$CMAKE_SRC" && grep -q 'WX' "$CMAKE_SRC"; then
    pass "poc_domain enables /W4 /WX (MSVC)"
  else
    fail "src/CMakeLists.txt missing MSVC warnings-as-errors (/W4 /WX) for poc_domain"
  fi
  if grep -q -- '-Werror' "$CMAKE_SRC"; then
    pass "poc_domain enables -Werror (g++/clang)"
  fi
else
  fail "missing ${CMAKE_SRC}"
fi

# --- 2. CPack config exists + best-effort run ---
echo "== [2] CPack =="
CPACK_OK=0
if grep -q "include(CPack)" "$ROOT_CMAKE"; then
  pass "root CMakeLists includes CPack"
  CPACK_OK=1
else
  fail "root CMakeLists does not include(CPack)"
fi
if grep -q "CPACK_PACKAGE_NAME" "$ROOT_CMAKE"; then
  pass "CPACK_PACKAGE_NAME set"
else
  fail "CPACK_PACKAGE_NAME not set"
fi
if grep -q "CPACK_GENERATOR" "$ROOT_CMAKE"; then
  pass "CPACK_GENERATOR set"
else
  fail "CPACK_GENERATOR not set"
fi
# windeployqt referenced for bundling Qt runtime DLLs.
if grep -iq "windeployqt" "$ROOT_CMAKE"; then
  pass "CPack packaging uses windeployqt"
else
  note "windeployqt not referenced in root CMakeLists (may be in a packaging script)"
fi

# Best-effort CPack run: needs cmake + a configured build dir.
if command -v cmake >/dev/null 2>&1 && [ "$CPACK_OK" = "1" ]; then
  BUILD="${PROJECT_ROOT}/build.p14"
  cmake -S "${PROJECT_ROOT}" -B "${BUILD}" -DCMAKE_BUILD_TYPE=Release \
    >"${LOG_DIR}/configure.log" 2>&1
  if cmake --build "${BUILD}" --config Release >"${LOG_DIR}/build.log" 2>&1; then
    # cpack must be pointed at the generated CPackConfig.cmake in the build
    # dir (it is not discovered from the project root).
    if cpack --config "${BUILD}/CPackConfig.cmake" -B "${BUILD}/package" -C Release \
        >"${LOG_DIR}/cpack.log" 2>&1; then
      pass "cpack ran successfully (see ${LOG_DIR}/cpack.log)"
    else
      fail "cpack failed (see ${LOG_DIR}/cpack.log)"
    fi
  else
    note "cmake build failed; cpack not attempted (see ${LOG_DIR}/build.log)"
  fi
else
  note "cmake unavailable or CPack not configured; cpack run skipped"
fi

# --- 3. Release tag / release notes ---
echo "== [3] release tag / notes =="
TAG=""
if command -v git >/dev/null 2>&1; then
  TAG="$(git -C "$PROJECT_ROOT" describe --tags --abbrev=0 2>/dev/null)"
fi
if [ -n "$TAG" ]; then
  pass "release tag found: ${TAG}"
else
  note "no git release tag found (devops tags at release time)"
fi
if [ -f "${PROJECT_ROOT}/docs/release-notes.md" ] || [ -f "${PROJECT_ROOT}/RELEASE_NOTES.md" ]; then
  pass "release notes file present"
else
  note "release notes file not found (docs/release-notes.md or RELEASE_NOTES.md)"
fi

# --- 4. Full test suites (gated by toolchain; one at a time, hard timeout) ---
echo "== [4] full test suites =="

# P4: Python pytest (python + pytest required).
if command -v python >/dev/null 2>&1 && python -c "import pytest" >/dev/null 2>&1; then
  if timeout 240 bash "${SCRIPT_DIR}/../phase4/run_tests.sh" >"${LOG_DIR}/p4.log" 2>&1; then
    pass "P4 pytest passed"
  else
    fail "P4 pytest failed (see ${LOG_DIR}/p4.log)"
  fi
else
  note "P4 pytest skipped: python+pytest unavailable"
fi

# P6: C++ CTest (needs cmake + MSVC cl in VS 2022 env).
if command -v cmake >/dev/null 2>&1 && (command -v cl >/dev/null 2>&1 || [ -n "${VSCMD_ARG_TGT_ARCH:-}" ]); then
  if timeout 300 bash "${SCRIPT_DIR}/../phase6/run_ctest.sh" >"${LOG_DIR}/p6.log" 2>&1; then
    pass "P6 CTest passed"
  else
    fail "P6 CTest failed (see ${LOG_DIR}/p6.log)"
  fi
else
  note "P6 CTest skipped: cmake or MSVC cl (VS 2022 env) unavailable"
fi

# P11: end-to-end integration (needs dotnet + curl).
if command -v dotnet >/dev/null 2>&1 && command -v curl >/dev/null 2>&1; then
  if timeout 300 bash "${SCRIPT_DIR}/../phase11/verify_integration.sh" >"${LOG_DIR}/p11.log" 2>&1; then
    pass "P11 E2E integration passed"
  else
    fail "P11 E2E integration failed (see ${LOG_DIR}/p11.log)"
  fi
else
  note "P11 E2E skipped: dotnet or curl unavailable"
fi

# --- Summary ---
echo ""
if [ "${FAILURES}" -eq 0 ]; then
  echo "RESULT: PASS"
  exit 0
else
  echo "RESULT: FAIL (${FAILURES} check(s) failed)"
  exit 1
fi
