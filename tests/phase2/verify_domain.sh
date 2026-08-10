#!/usr/bin/env bash
#
# Phase 2 build-verification test (scrum-master authored).
#
# Verifies the P2 domain model:
#   1. The `poc_domain` CMake target builds (integration with phase-1 build).
#   2. The domain API check program compiles against the implementation.
#   3. The check program runs and passes all validation assertions.
#
# Compiler-agnostic: uses MSVC `cl` when available (e.g. inside a Visual Studio
# Developer environment), otherwise a g++-style compiler.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 300 bash tests/phase2/verify_domain.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
WORK_DIR="${BUILD_DIR}.p2"
SRC_DOMAIN="${PROJECT_ROOT}/src/domain"
CHECK_SRC="${SCRIPT_DIR}/domain_check.cpp"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }

# --- 0. Tooling preflight (cl or g++-style compiler) ---
CL=""
if command -v cl >/dev/null 2>&1; then CL="cl"; fi

CXX=""
for cand in c++ g++ clang++; do
  if command -v "$cand" >/dev/null 2>&1; then CXX="$cand"; break; fi
done

if [ -n "$CL" ]; then
  echo "Compiler: MSVC cl"
elif [ -n "$CXX" ]; then
  echo "Compiler: $CXX"
else
  echo "ERROR: no C++ compiler found (need MSVC `cl` or g++-style)."
  exit 2
fi

# --- 1. Source files exist ---
echo "== [1] Domain sources =="
if [ -f "${SRC_DOMAIN}/domain.hpp" ]; then pass "domain.hpp exists"; else fail "domain.hpp missing"; fi
if [ -f "${SRC_DOMAIN}/domain.cpp" ]; then pass "domain.cpp exists"; else fail "domain.cpp missing"; fi

# --- 2. Build poc_domain target via cmake ---
echo "== [2] cmake build poc_domain =="
if [ -f "${PROJECT_ROOT}/CMakeLists.txt" ]; then
  if cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" >"${BUILD_DIR}.p2.configure.log" 2>&1 \
     && cmake --build "${BUILD_DIR}" --target poc_domain >"${BUILD_DIR}.p2.build.log" 2>&1; then
    pass "poc_domain target built"
  else
    fail "poc_domain target build failed (see ${BUILD_DIR}.p2.*.log)"
  fi
else
  fail "root CMakeLists.txt missing (phase 1 incomplete)"
fi

# --- 3. Compile + run the API check program ---
echo "== [3] compile + run domain check =="
mkdir -p "${WORK_DIR}"

COMPILED=0
if [ -n "$CL" ]; then
  CHECK_BIN="${WORK_DIR}/domain_check.exe"
  # cl needs Windows-style paths and must not have /flags mangled by git-bash.
  WIN_SRC="$(cygpath -w "${PROJECT_ROOT}/src")"
  WIN_WORK="$(cygpath -w "${WORK_DIR}")"
  WIN_CHECK="$(cygpath -w "${CHECK_SRC}")"
  WIN_DOMAIN="$(cygpath -w "${SRC_DOMAIN}/domain.cpp")"
  if MSYS_NO_PATHCONV=1 "$CL" /nologo /std:c++17 /EHsc \
           "/I${WIN_SRC}" "/Fo${WIN_WORK}\\" "/Fe${WIN_WORK}\\domain_check.exe" \
           "${WIN_CHECK}" "${WIN_DOMAIN}" \
           >"${BUILD_DIR}.p2.compile.log" 2>&1; then
    COMPILED=1
    pass "domain check compiled (cl)"
  else
    fail "domain check compilation failed (see ${BUILD_DIR}.p2.compile.log)"
  fi
else
  CHECK_BIN="${WORK_DIR}/domain_check"
  if "$CXX" -std=c++17 -Wall -Wextra -I"${PROJECT_ROOT}/src" \
         "${CHECK_SRC}" "${SRC_DOMAIN}/domain.cpp" -o "${CHECK_BIN}" \
         >"${BUILD_DIR}.p2.compile.log" 2>&1; then
    COMPILED=1
    pass "domain check compiled (${CXX})"
  else
    fail "domain check compilation failed (see ${BUILD_DIR}.p2.compile.log)"
  fi
fi

if [ "${COMPILED}" -eq 1 ] && [ -f "${CHECK_BIN}" ]; then
  if "${CHECK_BIN}" >"${BUILD_DIR}.p2.check.log" 2>&1; then
    pass "domain check passed ($(grep -c '^ok:' "${BUILD_DIR}.p2.check.log") assertions)"
  else
    fail "domain check runtime assertions failed"
    sed 's/^/    /' "${BUILD_DIR}.p2.check.log"
  fi
else
  fail "domain check binary was not produced"
fi

echo ""
if [ "${FAILURES}" -eq 0 ]; then
  echo "RESULT: PASS"
  exit 0
else
  echo "RESULT: FAIL (${FAILURES} check(s) failed)"
  exit 1
fi
