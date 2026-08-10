# PatchOrchestrator — bundles the Qt runtime DLLs next to each installed
# executable using windeployqt, BEFORE cpack packages the install tree.
#
# This script is invoked via install(SCRIPT) so it runs during the install
# step (which cpack performs into its staging directory before packaging).
# It is best-effort: if windeployqt.exe cannot be located, it warns and
# continues so the package can still be produced (without the Qt runtime).

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    return()
endif()

# --- Locate the Qt bin directory containing windeployqt.exe ---------------
set(QT_BIN "")
foreach(_cand
    "C:/Qt/6.8.2/msvc2022_64/bin"
    "$ENV{QT_ROOT}/bin"
    "$ENV{QT_DIR}/bin")
    if(EXISTS "${_cand}/windeployqt.exe")
        set(QT_BIN "${_cand}")
        break()
    endif()
endforeach()

if(NOT QT_BIN)
    message(WARNING
        "windeployqt.exe not found; Qt runtime DLLs will NOT be bundled. "
        "Install the package on a machine with Qt installed, or set QT_ROOT/"
        "QT_DIR and re-run the packaging step.")
    return()
endif()

set(BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
if(NOT EXISTS "${BIN_DIR}")
    message(WARNING "install bin dir not found: ${BIN_DIR}; skipping windeployqt.")
    return()
endif()

foreach(_exe
    patchorchestrator.exe
    patchorchestrator_ui.exe
    patchorchestrator_schedule_ui.exe
    patchorchestrator_control_ui.exe)
    set(_exe_path "${BIN_DIR}/${_exe}")
    if(NOT EXISTS "${_exe_path}")
        continue()
    endif()
    execute_process(
        COMMAND "${QT_BIN}/windeployqt.exe"
                --no-translations --no-system-d3d-compiler
                "${_exe_path}"
        RESULT_VARIABLE _windeployqt_rc)
    if(NOT _windeployqt_rc EQUAL 0)
        message(WARNING "windeployqt reported an issue for ${_exe} (rc=${_windeployqt_rc})")
    else()
        message(STATUS "Bundled Qt runtime for ${_exe}")
    endif()
endforeach()
