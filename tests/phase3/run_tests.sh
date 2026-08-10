#!/usr/bin/env bash
#
# Phase 3 verification runner (scrum-master authored).
# Runs the pytest suite in tests/phase3 against the Python simulation engine.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 180 bash tests/phase3/run_tests.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
PYTHON_DIR="${PROJECT_ROOT}/python"
LOG="${SCRIPT_DIR}/pytest.log"

# --- Preflight ---
if ! command -v python >/dev/null 2>&1 && ! command -v python3 >/dev/null 2>&1; then
  echo "ERROR: python not found on PATH."
  exit 2
fi
PY=python3; command -v python3 >/dev/null 2>&1 || PY=python

if ! "${PY}" -c "import pytest" >/dev/null 2>&1; then
  echo "ERROR: pytest not installed for ${PY}. Run: ${PY} -m pip install pytest"
  exit 2
fi
echo "pytest: $("${PY}" -m pytest --version)"

# --- Engine module present ---
if [ ! -f "${PYTHON_DIR}/engine.py" ]; then
  echo "FAIL: ${PYTHON_DIR}/engine.py missing (phase 3 not implemented)"
  exit 1
fi

# --- Run suite (PYTHONPATH points at python/ so `import engine` works) ---
echo "Running pytest suite..."
PYTHONPATH="${PYTHON_DIR}" "${PY}" -m pytest -q "${SCRIPT_DIR}" >"${LOG}" 2>&1
RC=$?

cat "${LOG}"
if [ "${RC}" -eq 0 ]; then
  echo "RESULT: PASS"
  exit 0
else
  echo "RESULT: FAIL"
  exit 1
fi
