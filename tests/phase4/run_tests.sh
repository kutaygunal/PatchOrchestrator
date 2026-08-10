#!/usr/bin/env bash
#
# Phase 4 verification runner (scrum-master authored).
# Runs the pytest suite in tests/phase4 against the Python simulation engine.
#
# NOTE: on this machine `python3` is a Windows Store alias WITHOUT pytest; the
# `python` interpreter (3.11) HAS pytest. This runner PREFERS `python`.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 180 bash tests/phase4/run_tests.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
PYTHON_DIR="${PROJECT_ROOT}/python"
LOG="${SCRIPT_DIR}/pytest.log"

# --- Preflight: prefer `python`, fall back to `python3` ---
PY=""
for cand in python python3; do
  if command -v "$cand" >/dev/null 2>&1; then
    if "$cand" -c "import pytest" >/dev/null 2>&1; then
      PY="$cand"
      break
    fi
  fi
done

if [ -z "$PY" ]; then
  echo "ERROR: no python interpreter with pytest found (tried 'python' then 'python3')."
  echo "       Install pytest: python -m pip install pytest"
  exit 2
fi
echo "pytest: $("${PY}" -m pytest --version)"

# --- Engine module present ---
if [ ! -f "${PYTHON_DIR}/engine.py" ]; then
  echo "FAIL: ${PYTHON_DIR}/engine.py missing (phase 3 not implemented)"
  exit 1
fi

# --- At least one test file must exist ---
if ! ls "${SCRIPT_DIR}"/test_*.py >/dev/null 2>&1; then
  echo "FAIL: no test_*.py files found in ${SCRIPT_DIR} (phase 4 not implemented)"
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
