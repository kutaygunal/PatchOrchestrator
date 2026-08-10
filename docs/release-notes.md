# PatchOrchestrator — Release Notes

## v0.1.0 (initial release)

Initial feature-complete release of PatchOrchestrator, a Qt-based control plane for
scheduling, pausing, and rolling back fleet-wide software patches.

### Highlights

- **C++/Qt desktop GUIs** (`src/ui/`): a read-only dashboard, a schedule-definition
  editor, and a control panel with Schedule / Pause / Resume / Rollback actions — all
  wired to the .NET API over HTTP.
- **.NET REST API** (`dotnet/PatchOrchestrator.Api/`): schedule management, pause /
  resume / rollback transitions, status queries, and a `/simulate` endpoint that drives
  the Python engine over a JSON subprocess bridge. OpenAPI/Swagger served at `/swagger`.
- **Python simulation engine** (`python/engine.py` + `python/bridge.py`): a deterministic,
  seeded endpoint patch state machine, so a fixed configuration and seed always reproduce
  the same rollout.

### Polish & hardening (Phase 14)

- **Structured logging** across all three layers (timestamp + level + message):
  - C++/Qt GUIs write structured records via `src/ui/log.hpp` and consistently handle
    API failures with status + log feedback, plus graceful shutdown (poll timer stopped
    on window close).
  - .NET API logs every request and the Python bridge call through Microsoft.Extensions
    Logging, and returns a clean 500 on simulation-engine failure.
  - Python `bridge.py` writes structured logs to stderr (keeping stdout a pure JSON
    contract) and surfaces clear `{"error": ...}` messages with non-zero exit codes.
- **Packaging / installer:** CPack target added to the root `CMakeLists.txt`
  (`CPACK_PACKAGE_NAME`, `CPACK_PACKAGE_VERSION`, `CPACK_GENERATOR`), and a
  `windeployqt` install step (`cmake/bundle_qt.cmake`) bundles the Qt runtime DLLs into
  the install tree before `cpack` produces a redistributable ZIP.
- **Full test suite confirmed:** P4 (pytest engine), P6 (CTest/gtest domain), and P11
  (end-to-end GUI → API → engine) all pass, plus the .NET API/bridge suites.

### Build & package

```bash
cmake -S . -B build.p14 -DCMAKE_BUILD_TYPE=Release
cmake --build build.p14 --config Release
cpack -B build.p14/package -C Release
```

The packaged installer is produced under `build.p14/package/`.

### Notes

- The GUI binaries are intended for Windows with the bundled Qt runtime (via
  `windeployqt`). Headless/smoke runs use `QT_QPA_PLATFORM=offscreen`.
- The `.NET API` and `Python engine` remain run-from-source in this release.
