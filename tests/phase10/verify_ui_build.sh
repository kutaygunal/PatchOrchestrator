#!/usr/bin/env bash
#
# Phase 10 verification runner (scrum-master authored).
# Builds the Qt control-panel target `patchorchestrator_control_ui`, confirms
# it links, and runs an offscreen smoke check.
#
# Compiler-agnostic: works with MSVC `cl` (inside the VS 2022 developer
# environment) or a g++-style compiler.
#
# On this machine there is NO g++/clang on PATH. MSVC `cl` is only available
# inside the VS 2022 developer environment. Run this runner inside it:
#
#   cmd /c "call \"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat\" && bash tests/phase10/verify_ui_build.sh"
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 300 bash tests/phase10/verify_ui_build.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Native Windows tools (cmake, MSVC) cannot consume MSYS-style paths such as
# /c/Users/... Convert to a Windows path when cygpath is available so the
# runner works both from a native cmd prompt and from a git-bash/MSYS shell.
if command -v cygpath >/dev/null 2>&1; then
    PROJECT_ROOT="$(cygpath -w "${PROJECT_ROOT}")"
fi

BUILD_DIR="${PROJECT_ROOT}/build.p10"
LOG_DIR="${BUILD_DIR}"
mkdir -p "${LOG_DIR}"
TARGET="patchorchestrator_control_ui"
EXE_NAME="patchorchestrator_control_ui"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }

# --- 0. Preflight ---
if ! command -v cmake >/dev/null 2>&1; then
  echo "ERROR: cmake not found on PATH."
  exit 2
fi
echo "cmake: $(cmake --version | head -n1)"

# --- 1. Configure ---
echo "== [1] cmake configure =="
if cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" >"${LOG_DIR}/configure.log" 2>&1; then
  pass "cmake configure succeeded"
else
  fail "cmake configure failed (see ${LOG_DIR}/configure.log)"
fi

# --- 2. Build the target ---
echo "== [2] cmake build ${TARGET} =="
if cmake --build "${BUILD_DIR}" --target "${TARGET}" --config Release \
     >"${LOG_DIR}/build.log" 2>&1; then
  pass "${TARGET} built"
else
  fail "${TARGET} build failed (see ${LOG_DIR}/build.log)"
fi

# --- 3. Confirm the executable was produced (links) ---
echo "== [3] executable produced =="
EXE="$(find "${BUILD_DIR}" -type f -name "${EXE_NAME}.exe" 2>/dev/null | head -n1)"
if [ -z "$EXE" ]; then
  EXE="$(find "${BUILD_DIR}" -type f -name "${EXE_NAME}" 2>/dev/null | head -n1)"
fi
if [ -n "$EXE" ] && [ -f "$EXE" ]; then
  pass "executable produced: ${EXE}"
else
  fail "no executable '${EXE_NAME}' produced under ${BUILD_DIR}"
fi

# --- 4. Locate Qt bin dir (for DLLs + plugins) ---
echo "== [4] locate Qt =="
QT_BIN=""
# Prefer the Qt6_DIR from the CMake cache: <root>/lib/cmake/Qt6 -> <root>/bin.
QT6_DIR="$(grep -E '^Qt6_DIR:PATH=' "${BUILD_DIR}/CMakeCache.txt" 2>/dev/null | head -n1 | cut -d= -f2)"
if [ -n "$QT6_DIR" ]; then
  QT_ROOT="$(cd "$(dirname "$(dirname "$(dirname "$QT6_DIR")")")" 2>/dev/null && pwd)"
  QT_BIN="${QT_ROOT}/bin"
fi
# Fall back to a known install path if the cache lookup failed.
if [ -z "$QT_BIN" ] || [ ! -d "$QT_BIN" ]; then
  for cand in "C:/Qt/6.8.2/msvc2022_64/bin" "$QT_ROOT/bin"; do
    if [ -d "$cand" ]; then QT_BIN="$cand"; break; fi
  done
fi
if [ -n "$QT_BIN" ] && [ -d "$QT_BIN" ]; then
  pass "Qt bin dir: ${QT_BIN}"
else
  fail "could not locate Qt bin dir (Qt DLLs may be missing at runtime)"
fi

# --- 5. Deploy Qt DLLs next to the exe (windeployqt) ---
echo "== [5] deploy Qt DLLs =="
if [ -n "${EXE:-}" ] && [ -f "$EXE" ] && [ -n "$QT_BIN" ] && [ -d "$QT_BIN" ]; then
  if "${QT_BIN}/windeployqt.exe" --no-translations --no-system-d3d-compiler "$EXE" \
       >"${LOG_DIR}/deploy.log" 2>&1; then
    pass "windeployqt deployed Qt DLLs next to exe"
  else
    fail "windeployqt failed (see ${LOG_DIR}/deploy.log)"
  fi
else
  fail "cannot deploy: exe or Qt bin dir not found"
fi

# --- 6. Offscreen smoke run (Qt bin on PATH + plugin path) ---
echo "== [6] offscreen smoke run =="
if [ -n "${EXE:-}" ] && [ -f "$EXE" ]; then
  # Prepend Qt bin to PATH so the app can find Qt DLLs at runtime.
  if [ -n "$QT_BIN" ] && [ -d "$QT_BIN" ]; then
    export PATH="${QT_BIN}:${PATH}"
    export QT_PLUGIN_PATH="${QT_ROOT}/plugins"
  fi
  QT_QPA_PLATFORM=offscreen timeout 5 "$EXE" >"${LOG_DIR}/smoke.log" 2>&1
  rc=$?
  if [ "$rc" -eq 124 ]; then
    pass "app started and ran (killed by timeout, no crash)"
  elif [ "$rc" -eq 0 ]; then
    pass "app started and exited cleanly"
  else
    fail "app crashed on startup (rc=$rc)"
    sed 's/^/    /' "${LOG_DIR}/smoke.log"
  fi
else
  fail "cannot smoke-run: executable not found"
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
