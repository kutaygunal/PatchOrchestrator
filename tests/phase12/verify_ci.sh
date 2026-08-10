#!/usr/bin/env bash
#
# Phase 12 verification runner (scrum-master authored).
# Locally "dry-runs" the CI/CD config: validates that .github/workflows/ci.yml
# is parseable YAML and that it references the right build targets, test
# suites, Qt install action, and artifact upload. Also confirms the underlying
# test runner scripts exist and that CMake declares all five targets.
#
# Full CI only runs on GitHub (windows-latest); this harness verifies the
# config is coherent without executing the pipeline.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 120 bash tests/phase12/verify_ci.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
CI_YML="${PROJECT_ROOT}/.github/workflows/ci.yml"
LOG_DIR="${PROJECT_ROOT}/build.p12"
mkdir -p "${LOG_DIR}"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }

# --- 0. Preflight ---
echo "== [0] preflight =="
if [ ! -f "$CI_YML" ]; then
  echo "ERROR: ${CI_YML} missing (phase 1 not implemented)."
  exit 2
fi
pass "ci.yml exists"

# --- 1. YAML parseable (python yaml if available, else minimal structural check) ---
echo "== [1] YAML parseable =="
YAML_OK=0
if command -v python >/dev/null 2>&1 && python -c "import yaml" >/dev/null 2>&1; then
  if python - "$CI_YML" <<'PY' 2>"${LOG_DIR}/yaml.log"
import sys, yaml
with open(sys.argv[1], 'r', encoding='utf-8') as f:
    data = yaml.safe_load(f)
assert isinstance(data, dict) and 'jobs' in data, "missing top-level 'jobs'"
PY
  then
    YAML_OK=1
    pass "ci.yml is valid YAML (PyYAML)"
  else
    fail "ci.yml failed YAML parse (see ${LOG_DIR}/yaml.log)"
  fi
else
  note "PyYAML not available; falling back to structural grep check."
  if grep -q '^jobs:' "$CI_YML" && grep -q 'runs-on:' "$CI_YML" && grep -q 'steps:' "$CI_YML"; then
    YAML_OK=1
    pass "ci.yml has jobs/runs-on/steps structure (grep fallback)"
  else
    fail "ci.yml lacks jobs/runs-on/steps structure"
  fi
fi

# --- 2. Required workflow features present ---
echo "== [2] workflow features =="
grep_q() { grep -q "$1" "$CI_YML"; }

# Qt install action
if grep_q 'install-qt-action'; then pass "uses jurplel/install-qt-action"; else fail "missing install-qt-action"; fi
if grep_q "6.8.2"; then pass "Qt version 6.8.2"; else fail "missing Qt version 6.8.2"; fi
if grep_q 'win64_msvc2022_64'; then pass "Qt arch win64_msvc2022_64"; else fail "missing Qt arch win64_msvc2022_64"; fi

# Setup .NET for the API / E2E suite
if grep_q 'setup-dotnet'; then pass "uses actions/setup-dotnet"; else fail "missing actions/setup-dotnet"; fi

# All five CMake targets must be referenced (build-all or explicit target list)
TARGETS="patchorchestrator poc_domain patchorchestrator_ui patchorchestrator_schedule_ui patchorchestrator_control_ui"
for t in $TARGETS; do
  if grep_q "$t"; then pass "references target: $t"; else fail "workflow does not mention target: $t"; fi
done

# BUILD_TESTING=ON so CTest/gtest is built
if grep_q 'BUILD_TESTING=ON'; then pass "configure enables BUILD_TESTING=ON"; else fail "missing BUILD_TESTING=ON in configure"; fi

# Test suites: P4 pytest, P6 ctest, P11 E2E
if grep_q 'pytest'; then pass "references pytest (P4)"; else fail "missing pytest step (P4)"; fi
if grep_q 'ctest'; then pass "references ctest (P6)"; else fail "missing ctest step (P6)"; fi
if grep_q 'phase11'; then pass "references phase11 E2E (P11)"; else fail "missing phase11 E2E step"; fi

# Artifact upload
if grep_q 'upload-artifact'; then pass "references actions/upload-artifact"; else fail "missing artifact upload step"; fi

# --- 3. Underlying test runner scripts exist ---
echo "== [3] underlying runners exist =="
for r in \
  "tests/phase4/run_tests.sh" \
  "tests/phase6/run_ctest.sh" \
  "tests/phase5/verify_api.sh" \
  "tests/phase7/verify_bridge.sh" \
  "tests/phase11/verify_integration.sh" \
  "tests/phase11/run_p11.bat"; do
  if [ -f "${PROJECT_ROOT}/${r}" ]; then
    pass "runner exists: ${r}"
  else
    fail "runner missing: ${r}"
  fi
done

# --- 4. CMake declares all five targets ---
echo "== [4] CMake declares targets =="
CMAKE_SRC="${PROJECT_ROOT}/src/CMakeLists.txt"
if [ -f "$CMAKE_SRC" ]; then
  for t in $TARGETS; do
    if grep -q "${t}" "$CMAKE_SRC"; then
      pass "CMake declares target: ${t}"
    else
      fail "CMake missing target declaration: ${t}"
    fi
  done
else
  fail "missing ${CMAKE_SRC}"
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
