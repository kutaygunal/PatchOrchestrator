#!/usr/bin/env bash
#
# Phase 13 verification runner (scrum-master authored).
# Checks that the top-level README.md exists and contains the required
# documentation sections (overview, architecture, prerequisites, build/run for
# the C++/Qt GUI, .NET API, and Python engine, test instructions, and
# screenshots). Optionally confirms referenced screenshots exist in the repo.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 60 bash tests/phase13/verify_readme.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
README="${PROJECT_ROOT}/README.md"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }

# --- 0. README exists ---
echo "== [0] README exists =="
if [ ! -f "$README" ]; then
  echo "FAIL: ${README} missing (phase 13 not implemented)."
  exit 1
fi
pass "README.md exists"

# --- 1. Required sections present (case-insensitive keyword match) ---
echo "== [1] required sections =="
has() { grep -iq "$1" "$README"; }

# Overview
if has "overview" || has "introduction" || has "what is"; then
  pass "overview section"
else
  fail "missing overview/introduction section"
fi

# Architecture
if has "architecture"; then
  pass "architecture section"
else
  fail "missing architecture section"
fi

# Prerequisites
if has "prerequisite"; then
  pass "prerequisites section"
else
  fail "missing prerequisites section"
fi

# Build & run: C++/Qt GUI
if has "cmake" && has "qt"; then
  pass "C++/Qt build & run instructions (cmake + qt)"
else
  fail "missing C++/Qt build/run instructions (cmake/qt)"
fi

# Build & run: .NET API
if has "dotnet" && has "PATCHORCH_PYTHON_DIR"; then
  pass ".NET API build/run instructions (dotnet + PATCHORCH_PYTHON_DIR)"
else
  fail "missing .NET API build/run instructions (dotnet/PATCHORCH_PYTHON_DIR)"
fi

# Build & run: Python engine
if has "python" && has "engine"; then
  pass "Python engine build/run instructions (python/engine)"
else
  fail "missing Python engine instructions (python/engine)"
fi

# Test instructions
if has "pytest" && has "ctest"; then
  pass "test instructions (pytest + ctest)"
else
  fail "missing test instructions (pytest/ctest)"
fi

# Screenshot reference
if has "screenshot"; then
  pass "screenshot section/reference present"
else
  fail "missing screenshot reference"
fi

# --- 2. Referenced screenshots exist (best-effort) ---
echo "== [2] referenced screenshots exist =="
# Extract image references like ![alt](path.png) or ![alt](path "title").
SHOTS="$(grep -oE '!\[[^]]*\]\([^)]+\.(png|jpg|jpeg|gif|webp)' "$README" \
  | sed -E 's/!\[[^]]*\]\(//' )"
if [ -z "$SHOTS" ]; then
  # Fallback: bare .png/.jpg paths mentioned.
  SHOTS="$(grep -oE '[^ ()]+\.(png|jpg|jpeg|gif|webp)' "$README")"
fi

if [ -z "$SHOTS" ]; then
  fail "no screenshot file path referenced in README"
else
  FOUND=0
  for s in $SHOTS; do
    s="${s%\"}"; s="${s#\"}"   # strip surrounding quotes
    case "$s" in
      http*) note "skip remote image (not checked locally): $s"; FOUND=1 ;;
      *)
        if [ -f "${PROJECT_ROOT}/${s}" ] || [ -f "$s" ]; then
          pass "screenshot exists: $s"
          FOUND=1
        else
          fail "referenced screenshot missing: $s"
        fi
        ;;
    esac
  done
  if [ "$FOUND" = "0" ]; then
    fail "no locally-existing screenshot found"
  fi
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
