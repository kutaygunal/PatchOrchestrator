@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "PATH=C:\Qt\6.8.2\msvc2022_64\bin;%PATH%"
bash tests/phase8/verify_ui_build.sh
