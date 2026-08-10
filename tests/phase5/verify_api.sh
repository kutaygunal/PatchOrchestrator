#!/usr/bin/env bash
#
# Phase 5 verification harness (scrum-master authored).
# Builds the .NET REST API, runs it, and checks the REST contract endpoints.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 300 bash tests/phase5/verify_api.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
PROJECT="${PROJECT_ROOT}/dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj"
BASE_URL="${BASE_URL:-http://localhost:5000}"
LOG_DIR="${PROJECT_ROOT}/build.p5"
mkdir -p "${LOG_DIR}"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }

API_PID=""
cleanup() {
  if [ -n "$API_PID" ]; then
    # Kill the process tree (dotnet run spawns a child server process).
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

# --- 5. Contract checks ---
echo "== [5] contract checks =="
http_code() { curl -s -o /dev/null -w '%{http_code}' "$@"; }

# health body contains "ok"
body="$(curl -s "${BASE_URL}/api/health")"
if echo "$body" | grep -qi "ok"; then pass "health body contains 'ok'"; else fail "health body missing 'ok': $body"; fi

# create schedule -> 201, echoes id
code="$(http_code -X POST -H 'Content-Type: application/json' \
  -d '{"id":"sch-1","package":"pkg-v2","group_id":"grp-1"}' "${BASE_URL}/api/schedules")"
if [ "$code" = "201" ]; then pass "POST /api/schedules -> 201"; else fail "POST /api/schedules -> $code (expected 201)"; fi
body="$(curl -s -X POST -H 'Content-Type: application/json' \
  -d '{"id":"sch-1","package":"pkg-v2","group_id":"grp-1"}' "${BASE_URL}/api/schedules")"
if echo "$body" | grep -q "sch-1"; then pass "create schedule echoes id"; else fail "create schedule did not echo id: $body"; fi

# pause/resume/rollback -> 200
for op in pause resume rollback; do
  code="$(http_code -X POST "${BASE_URL}/api/schedules/sch-1/${op}")"
  if [ "$code" = "200" ]; then pass "POST /api/schedules/sch-1/${op} -> 200"; else fail "POST /api/schedules/sch-1/${op} -> $code (expected 200)"; fi
done

# status -> 200, body contains "status"
code="$(http_code "${BASE_URL}/api/schedules/sch-1/status")"
if [ "$code" = "200" ]; then pass "GET /api/schedules/sch-1/status -> 200"; else fail "GET status -> $code (expected 200)"; fi
body="$(curl -s "${BASE_URL}/api/schedules/sch-1/status")"
if echo "$body" | grep -qi "status"; then pass "status body contains 'status'"; else fail "status body missing 'status': $body"; fi

# unknown id -> 404
code="$(http_code "${BASE_URL}/api/schedules/nope/status")"
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
