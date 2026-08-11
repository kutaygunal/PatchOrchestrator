@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set PATH=C:\Qt\6.8.2\msvc2022_64\bin;%PATH%
set QT_QPA_PLATFORM=offscreen
build\tests\phase6\Release\p3_dashboard_discovery.exe
echo P3_EXIT=%ERRORLEVEL%
