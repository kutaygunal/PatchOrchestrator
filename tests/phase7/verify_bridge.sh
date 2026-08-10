#!/usr/bin/env bash
#
# Phase 7 verification harness (scrum-master authored).
# Builds the .NET API, runs it, and verifies the API <-> engine bridge
# end-to-end: the /api/schedules/{id}/simulate endpoint drives the Python
# engine and returns parsed results deterministically.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 300 bash tests/phase7/verify_bridge.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
PROJECT="${PROJECT_ROOT}/dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj"
BASE_URL="${BASE_URL:-http://localhost:5000}"
LOG_DIR="${PROJECT_ROOT}/build.p7"
mkdir -p "${LOG_DIR}"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }

API_PID=""
cleanup() {
  if [ -n "$API_PID" ]; then
    cmd //c "taskkill /F /T /PID ${API_PID}" >/dev/null 2>&1
    API_PID=""
  fi
}
trap cleanup EXIT

# --- 0. Preflight ---
if ! command -v dotnet >/dev/null 2>&1; then
  echo "ERROR: dotnet not found on PATH."
  exit 2
fi
echo "dotnet: $(dotnet --version)"
if ! command -v curl >/dev/null 2>&1; then
  echo "ERROR: curl not found on PATH."
  exit 2
fi

# --- 1. Project exists ---
echo "== [1] Project =="
if [ -f "$PROJECT" ]; then
  pass "project exists: ${PROJECT}"
else
  fail "project missing: ${PROJECT} (phase 5 not implemented)"
fi

# --- 2. Build ---
echo "== [2] dotnet build =="
if dotnet build "$PROJECT" -c Release >"${LOG_DIR}/build.log" 2>&1; then
  pass "dotnet build succeeded"
else
  fail "dotnet build failed (see ${LOG_DIR}/build.log)"
fi

# --- 3. Run API in background ---
echo "== [3] run API =="
ASPNETCORE_URLS="$BASE_URL" dotnet run --project "$PROJECT" -c Release --no-build \
  >"${LOG_DIR}/run.log" 2>&1 &
API_PID=$!

# --- 4. Wait for readiness (bounded) ---
echo "== [4] wait for readiness =="
READY=0
for i in $(seq 1 40); do
  code="$(curl -s -o /dev/null -w '%{http_code}' "${BASE_URL}/api/health" 2>/dev/null)"
  if [ "$code" = "200" ]; then READY=1; break; fi
  sleep 1
done
if [ "$READY" = "1" ]; then
  pass "API ready (health returned 200)"
else
  fail "API did not become ready within 40s (see ${LOG_DIR}/run.log)"
fi

# --- 5. Bridge contract checks ---
echo "== [5] bridge contract checks =="
http_code() { curl -s -o /dev/null -w '%{http_code}' "$@"; }

# create a schedule to simulate
code="$(http_code -X POST -H 'Content-Type: application/json' \
  -d '{"id":"sch-1","package":"pkg-v2","group_id":"grp-1"}' "${BASE_URL}/api/schedules")"
if [ "$code" = "201" ]; then pass "POST /api/schedules -> 201"; else fail "POST /api/schedules -> $code (expected 201)"; fi

SIM_BODY='{"seed":42,"endpoints":[{"id":"ep-1","failure_rate":0.1},{"id":"ep-2","failure_rate":0.0}]}'

# simulate -> 200, body contains endpoints + state
code="$(http_code -X POST -H 'Content-Type: application/json' -d "$SIM_BODY" \
  "${BASE_URL}/api/schedules/sch-1/simulate")"
if [ "$code" = "200" ]; then pass "POST /api/schedules/sch-1/simulate -> 200"; else fail "simulate -> $code (expected 200)"; fi

body="$(curl -s -X POST -H 'Content-Type: application/json' -d "$SIM_BODY" \
  "${BASE_URL}/api/schedules/sch-1/simulate")"
if echo "$body" | grep -q '"endpoints"'; then pass "simulate body contains 'endpoints'"; else fail "simulate body missing 'endpoints': $body"; fi
if echo "$body" | grep -qi '"state"'; then pass "simulate body contains 'state'"; else fail "simulate body missing 'state': $body"; fi
if echo "$body" | grep -q '"ep-1"'; then pass "simulate body contains endpoint id 'ep-1'"; else fail "simulate body missing 'ep-1': $body"; fi

# determinism: same body + same seed -> identical results
body1="$(curl -s -X POST -H 'Content-Type: application/json' -d "$SIM_BODY" \
  "${BASE_URL}/api/schedules/sch-1/simulate")"
body2="$(curl -s -X POST -H 'Content-Type: application/json' -d "$SIM_BODY" \
  "${BASE_URL}/api/schedules/sch-1/simulate")"
if [ "$body1" = "$body2" ]; then pass "simulate is deterministic (same seed -> identical)"; else fail "simulate not deterministic"; fi

# unknown id -> 404
code="$(http_code -X POST -H 'Content-Type: application/json' -d "$SIM_BODY" \
  "${BASE_URL}/api/schedules/nope/simulate")"
if [ "$code" = "404" ]; then pass "unknown id -> 404"; else fail "unknown id -> $code (expected 404)"; fi

# --- Summary ---
echo ""
if [ "${FAILURES}" -eq 0 ]; then
  echo "RESULT: PASS"
  exit 0
else
  echo "RESULT: FAIL (${FAILURES} check(s) failed)"
  exit 1
fi
