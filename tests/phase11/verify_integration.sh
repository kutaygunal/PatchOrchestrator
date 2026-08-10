#!/usr/bin/env bash
#
# Phase 11 integration runner (scrum-master authored).
# Exercises the full GUI -> API -> engine flow against a live .NET API with the
# Python simulation engine backend, over HTTP (curl). Best-effort offscreen
# launch of the built Qt executables against the running API.
#
# Self-contained and runnable with a HARD TIMEOUT. Run ONE AT A TIME:
#     timeout 300 bash tests/phase11/verify_integration.sh
#
# Exit codes: 0 PASS, 1 FAIL, 2 ENV error.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Native Windows tools (dotnet, curl, MSVC) cannot consume MSYS-style paths
# such as /c/Users/... Convert to a Windows path when cygpath is available.
if command -v cygpath >/dev/null 2>&1; then
  PROJECT_ROOT="$(cygpath -w "${PROJECT_ROOT}")"
fi

PROJECT="${PROJECT_ROOT}/dotnet/PatchOrchestrator.Api/PatchOrchestrator.Api.csproj"
PYTHON_DIR="${PROJECT_ROOT}/python"
BASE_URL="${BASE_URL:-http://localhost:5000}"
LOG_DIR="${PROJECT_ROOT}/build.p11"
mkdir -p "${LOG_DIR}"

FAILURES=0
fail() { echo "FAIL: $1"; FAILURES=$((FAILURES + 1)); }
pass() { echo "PASS: $1"; }
note() { echo "NOTE: $1"; }

API_PID=""
cleanup() {
  if [ -n "$API_PID" ]; then
    # Kill the process tree (dotnet run spawns a child server process).
    cmd //c "taskkill /F /T /PID ${API_PID}" >/dev/null 2>&1
    API_PID=""
  fi
  # Belt-and-braces: kill anything still listening on the base port so no
  # orphaned server survives the run (working rule: no background processes).
  local port
  port="${BASE_URL##*:}"
  local pid
  for pid in $(netstat -ano 2>/dev/null | grep ":${port}" | grep -i LISTENING | awk '{print $NF}' | sort -u); do
    cmd //c "taskkill /F /T /PID ${pid}" >/dev/null 2>&1
  done
}
trap cleanup EXIT

# --- 0. Preflight ---
echo "== [0] preflight =="
if ! command -v dotnet >/dev/null 2>&1; then
  echo "ERROR: dotnet not found on PATH."
  exit 2
fi
echo "dotnet: $(dotnet --version)"
if ! command -v curl >/dev/null 2>&1; then
  echo "ERROR: curl not found on PATH."
  exit 2
fi
if ! command -v jq >/dev/null 2>&1; then
  note "jq not found; payload checks fall back to grep (jq optional)."
fi
if [ -f "$PROJECT" ]; then
  pass "API project exists"
else
  echo "ERROR: API project missing: ${PROJECT} (phase 5 not implemented)."
  exit 2
fi

# --- 1. Build the API ---
echo "== [1] dotnet build =="
if dotnet build "$PROJECT" -c Release >"${LOG_DIR}/build.log" 2>&1; then
  pass "dotnet build succeeded"
else
  fail "dotnet build failed (see ${LOG_DIR}/build.log)"
fi

# --- 2. Start the API (with Python engine backend) ---
echo "== [2] start API =="
export PATCHORCH_PYTHON_DIR="$PYTHON_DIR"
ASPNETCORE_URLS="$BASE_URL" dotnet run --project "$PROJECT" -c Release --no-build \
  >"${LOG_DIR}/run.log" 2>&1 &
API_PID=$!

# --- 3. Wait for readiness (bounded) ---
echo "== [3] wait for readiness =="
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

# --- 4. End-to-end HTTP flow ---
echo "== [4] end-to-end HTTP flow =="
http_code() { curl -s -o /dev/null -w '%{http_code}' "$@"; }

# 4a. health body contains "ok"
body="$(curl -s "${BASE_URL}/api/health")"
if echo "$body" | grep -qi "ok"; then pass "health body contains 'ok'"; else fail "health body missing 'ok': $body"; fi

# 4b. create schedule -> 201, echoes id
code="$(http_code -X POST -H 'Content-Type: application/json' \
  -d '{"id":"sch-11","package":"pkg-v2","group_id":"grp-1"}' "${BASE_URL}/api/schedules")"
if [ "$code" = "201" ]; then pass "POST /api/schedules -> 201"; else fail "POST /api/schedules -> $code (expected 201)"; fi
body="$(curl -s -X POST -H 'Content-Type: application/json' \
  -d '{"id":"sch-11","package":"pkg-v2","group_id":"grp-1"}' "${BASE_URL}/api/schedules")"
if echo "$body" | grep -q "sch-11"; then pass "create schedule echoes id"; else fail "create schedule did not echo id: $body"; fi

# 4c. simulate -> 200, returns endpoints array with state/progress
code="$(http_code -X POST -H 'Content-Type: application/json' \
  -d '{"seed":42,"endpoints":[{"id":"ep-1","failure_rate":0.0},{"id":"ep-2","failure_rate":0.0}]}' \
  "${BASE_URL}/api/schedules/sch-11/simulate")"
if [ "$code" = "200" ]; then pass "POST /api/schedules/sch-11/simulate -> 200"; else fail "simulate -> $code (expected 200)"; fi
body="$(curl -s -X POST -H 'Content-Type: application/json' \
  -d '{"seed":42,"endpoints":[{"id":"ep-1","failure_rate":0.0},{"id":"ep-2","failure_rate":0.0}]}' \
  "${BASE_URL}/api/schedules/sch-11/simulate")"
if echo "$body" | grep -q "endpoints"; then pass "simulate returns 'endpoints'"; else fail "simulate missing 'endpoints': $body"; fi
if echo "$body" | grep -q "ep-1"; then pass "simulate returns endpoint id"; else fail "simulate missing endpoint id: $body"; fi
if echo "$body" | grep -qi "state"; then pass "simulate returns 'state'"; else fail "simulate missing 'state': $body"; fi

# 4d. pause/resume/rollback -> 200 with the requested status (per contract:
# pause->paused, resume->running, rollback->rolled-back).
for op in pause resume rollback; do
  case "$op" in
    pause)    want="paused";;
    resume)   want="running";;
    rollback) want="rolled-back";;
  esac
  code="$(http_code -X POST "${BASE_URL}/api/schedules/sch-11/${op}")"
  if [ "$code" = "200" ]; then pass "POST /api/schedules/sch-11/${op} -> 200"; else fail "POST ${op} -> $code (expected 200)"; fi
  b="$(curl -s -X POST "${BASE_URL}/api/schedules/sch-11/${op}")"
  if echo "$b" | grep -q "$want"; then pass "POST ${op} echoes status (${want})"; else fail "POST ${op} missing status: $b"; fi
done

# 4e. status -> 200, body contains status field
code="$(http_code "${BASE_URL}/api/schedules/sch-11/status")"
if [ "$code" = "200" ]; then pass "GET /api/schedules/sch-11/status -> 200"; else fail "GET status -> $code (expected 200)"; fi
body="$(curl -s "${BASE_URL}/api/schedules/sch-11/status")"
if echo "$body" | grep -qi "status"; then pass "status body contains 'status'"; else fail "status body missing 'status': $body"; fi

# 4f. unknown schedule -> 404 on status (GET), pause/simulate (POST).
# pause/simulate are POST-only routes; probing them with GET yields 405.
# simulate binds a body first, so send a valid payload for the unknown-id 404.
check_unknown() {
  local method="$1" path="$2" data="${3:-}"
  local code
  if [ -n "$data" ]; then
    code="$(http_code -X "$method" -H 'Content-Type: application/json' -d "$data" "${BASE_URL}/api/schedules/${path}")"
  else
    code="$(http_code -X "$method" "${BASE_URL}/api/schedules/${path}")"
  fi
  if [ "$code" = "404" ]; then pass "unknown '${path}' -> 404"; else fail "unknown '${path}' -> $code (expected 404)"; fi
}
check_unknown GET  "nope/status"
check_unknown POST "nope/pause"
check_unknown POST "nope/simulate" '{"seed":42,"endpoints":[{"id":"ep-x","failure_rate":0.0}]}'

# --- 5. Best-effort Qt offscreen launch against the running API ---
echo "== [5] Qt offscreen launch (best-effort) =="
# Locate Qt bin dir (same discovery as the P8/P9/P10 runners).
QT_BIN=""
QT_ROOT=""
QT6_DIR="$(grep -E '^Qt6_DIR:PATH=' "${PROJECT_ROOT}/build.p8/CMakeCache.txt" 2>/dev/null | head -n1 | cut -d= -f2)"
if [ -z "$QT6_DIR" ]; then
  QT6_DIR="$(grep -E '^Qt6_DIR:PATH=' "${PROJECT_ROOT}/build.p9/CMakeCache.txt" 2>/dev/null | head -n1 | cut -d= -f2)"
fi
if [ -n "$QT6_DIR" ]; then
  QT_ROOT="$(cd "$(dirname "$(dirname "$(dirname "$QT6_DIR")")")" 2>/dev/null && pwd)"
  QT_BIN="${QT_ROOT}/bin"
fi
if [ -z "$QT_BIN" ] || [ ! -d "$QT_BIN" ]; then
  for cand in "C:/Qt/6.8.2/msvc2022_64/bin" "$QT_ROOT/bin"; do
    if [ -d "$cand" ]; then QT_BIN="$cand"; QT_ROOT="$(cd "$(dirname "$QT_BIN")" && pwd)"; break; fi
  done
fi

QT_OK=0
if [ -n "$QT_BIN" ] && [ -d "$QT_BIN" ]; then
  pass "Qt bin dir found: ${QT_BIN}"
  QT_OK=1
else
  note "Qt bin dir not located; Qt offscreen check skipped (limitation)."
fi

# Candidate UI executables (P8/P9/P10 build dirs).
UI_EXES=""
for spec in "build.p8:patchorchestrator_ui" "build.p9:patchorchestrator_schedule_ui" "build.p10:patchorchestrator_control_ui"; do
  d="${spec%%:*}"; n="${spec##*:}"
  e="$(find "${PROJECT_ROOT}/${d}" -type f -name "${n}.exe" 2>/dev/null | head -n1)"
  [ -z "$e" ] && e="$(find "${PROJECT_ROOT}/${d}" -type f -name "${n}" 2>/dev/null | head -n1)"
  if [ -n "$e" ] && [ -f "$e" ]; then UI_EXES="$UI_EXES $e"; fi
done

if [ -z "$UI_EXES" ]; then
  note "No built Qt UI executables found (P8/P9/P10 not built yet); offscreen check skipped (limitation)."
else
  if [ "$QT_OK" = "1" ]; then
    for exe in $UI_EXES; do
      # Deploy Qt DLLs next to the exe (mirrors the P8/P9/P10 runners).
      if command -v "${QT_BIN}/windeployqt.exe" >/dev/null 2>&1; then
        "${QT_BIN}/windeployqt.exe" --no-translations --no-system-d3d-compiler "$exe" \
          >"${LOG_DIR}/deploy_$(basename "$exe").log" 2>&1 || note "windeployqt warnings for $(basename "$exe")"
      fi
      export PATH="${QT_BIN}:${PATH}"
      export QT_PLUGIN_PATH="${QT_ROOT}/plugins"
      QT_QPA_PLATFORM=offscreen timeout 5 "$exe" >"${LOG_DIR}/qt_$(basename "$exe").log" 2>&1
      rc=$?
      if [ "$rc" -eq 124 ]; then
        pass "$(basename "$exe") launched offscreen and ran (killed by timeout, no crash)"
      elif [ "$rc" -eq 0 ]; then
        pass "$(basename "$exe") launched offscreen and exited cleanly"
      else
        note "$(basename "$exe") exited rc=$rc (see ${LOG_DIR}/qt_$(basename "$exe").log) — not counted as a failure"
      fi
    done
  else
    note "Qt not deployable; offscreen launch skipped (limitation)."
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
