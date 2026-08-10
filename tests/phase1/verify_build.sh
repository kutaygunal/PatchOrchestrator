#!/usr/bin/env bash
#
# Phase 1 build-verification test (scrum-master authored).
#
# Verifies the PatchOrchestrator project skeleton + CMake build:
#   1. Expected top-level directory layout exists.
#   2. Root CMakeLists.txt exists and configures cleanly.
#   3. The project builds successfully.
#   4. The build produced a verifiable artifact (a non-empty file).
#
# Self-contained and runnable with a HARD TIMEOUT. Run it ONE AT A TIME:
#     timeout 300 bash tests/phase1/verify_build.sh
#
# Exit codes:
#   0  PASS
#   1  FAIL (a check did not pass)
#   2  USAGE/ENV error (e.g. cmake missing)

set -u

# --- Resolve project root (repo root = parent of tests/phase1) -----------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
ARTIFACT_DIR="${BUILD_DIR}/bin"

FAILURES=0

fail() {
  echo "FAIL: $1"
  FAILURES=$((FAILURES + 1))
}

pass() {
  echo "PASS: $1"
}

# --- 0. Tooling preflight ------------------------------------------------------
if ! command -v cmake >/dev/null 2>&1; then
  echo "ERROR: cmake not found on PATH. Cannot run build verification."
  exit 2
fi
if ! command -v cmake --version >/dev/null 2>&1; then
  echo "ERROR: cmake --version failed. cmake is broken."
  exit 2
fi
echo "cmake: $(cmake --version | head -n1)"

# --- 1. Expected directory layout ---------------------------------------------
echo "== [1] Directory layout =="
for d in src python dotnet tests docs; do
  if [ -d "${PROJECT_ROOT}/${d}" ]; then
    pass "directory '${d}' exists"
  else
    fail "directory '${d}' missing (expected at ${PROJECT_ROOT}/${d})"
  fi
done

# --- 2. Root CMakeLists.txt present -------------------------------------------
echo "== [2] Root CMakeLists.txt =="
if [ -f "${PROJECT_ROOT}/CMakeLists.txt" ]; then
  pass "root CMakeLists.txt exists"
else
  fail "root CMakeLists.txt missing"
fi

# --- 3. Configure -------------------------------------------------------------
echo "== [3] CMake configure =="
rm -rf "${BUILD_DIR}"
if cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release >"${BUILD_DIR}.configure.log" 2>&1; then
  pass "cmake configure succeeded"
else
  fail "cmake configure failed (see ${BUILD_DIR}.configure.log)"
fi

# --- 4. Build -----------------------------------------------------------------
echo "== [4] CMake build =="
if cmake --build "${BUILD_DIR}" --config Release >"${BUILD_DIR}.build.log" 2>&1; then
  pass "cmake build succeeded"
else
  fail "cmake build failed (see ${BUILD_DIR}.build.log)"
fi

# --- 5. Verifiable artifact ----------------------------------------------------
echo "== [5] Verifiable artifact =="
# A verifiable artifact is any non-empty regular file produced under the build
# tree (executable, library, or generated data). We require at least one.
ARTIFACT=""
if [ -d "${ARTIFACT_DIR}" ]; then
  ARTIFACT="$(find "${ARTIFACT_DIR}" -maxdepth 2 -type f -size +0c 2>/dev/null | head -n1)"
fi
if [ -z "${ARTIFACT}" ]; then
  # Fall back to any non-empty file anywhere in the build tree.
  ARTIFACT="$(find "${BUILD_DIR}" -type f -size +0c ! -name '*.log' 2>/dev/null | head -n1)"
fi

if [ -n "${ARTIFACT}" ]; then
  pass "verifiable artifact produced: ${ARTIFACT}"
else
  fail "no verifiable (non-empty) artifact produced under ${BUILD_DIR}"
fi

# --- Summary ------------------------------------------------------------------
echo ""
if [ "${FAILURES}" -eq 0 ]; then
  echo "RESULT: PASS"
  exit 0
else
  echo "RESULT: FAIL (${FAILURES} check(s) failed)"
  exit 1
fi
