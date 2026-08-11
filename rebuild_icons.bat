@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
cd /d C:\Users\kutay\Desktop\Projects\PatchOrchestrator
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target patchorchestrator_ui patchorchestrator_control_ui -- /m
echo BUILD_DONE
