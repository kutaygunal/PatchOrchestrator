@echo off
set "MSYSTEM="
set "MSYSTEM_CHOST="
set "MSYSTEM_PREFIX="
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set "PATH=C:\Qt\6.8.2\msvc2022_64\bin;%PATH%"
bash tests/phase9/verify_ui_build.sh
