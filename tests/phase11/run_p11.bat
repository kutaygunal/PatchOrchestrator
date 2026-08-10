@echo off
REM Phase 11 integration runner (Windows wrapper).
REM Drives the full GUI->API->engine flow against a live .NET API.
REM Run ONE at a time with a HARD TIMEOUT, inside the VS 2022 dev env
REM (only needed if the Qt offscreen check must load MSVC-built exes).

setlocal

set VC=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
if exist "%VC%" (
    call "%VC%"
) else (
    echo WARNING: vcvars64.bat not found; continuing without MSVC env.
)

set SCRIPT=%~dp0verify_integration.sh

if not exist "%SCRIPT%" (
    echo ERROR: missing runner %SCRIPT%
    exit /b 2
)

REM HARD timeout via timeout-for-kill style: run in foreground.
bash "%SCRIPT%"
exit /b %ERRORLEVEL%
