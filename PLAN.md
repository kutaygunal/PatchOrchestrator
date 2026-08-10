# Plan

Purpose: PatchOrchestrator: a Qt-based control plane for scheduling, pausing, and rolling back fleet-wide software patches. C++/Qt GUI, Python backend simulation, .NET API. Built to demonstrate C++/Qt cross-platform engineering for the NinjaOne Senior C++ Patching job.

## Phases (dependency-ordered)

| # | Phase | Description | Priority | Status | Assigned to | Tests | Committed |
|---|-------|-------------|----------|--------|-------------|-------|-----------|
| 1 | C++ skeleton + CMake build | DONE | PASS |
| 2 | Domain model (C++ core) | Fleet/Endpoint/Group, PatchSchedule, MaintenanceWindow, RolloutStage plain C++ data types + factory/validation, no GUI | High | TODO | - | - | - |
| 3 | Python simulation engine | Endpoint patch state machine (pending/running/paused/failed/rolled-back), progress + failure + rollback modeling, deterministic seeded sim | High | TODO | - | - | - |
| 4 | Simulation engine unit tests | pytest suite covering progress, failures, pause/resume, rollback, boundary cases | High | TODO | - | - | - |
| 5 | .NET REST API boundary | ASP.NET Core REST endpoints for schedule, pause/resume, rollback, status query; documented contract/OpenAPI | High | TODO | - | - | - |
| 6 | C++ core unit tests | CTest/gtest coverage of domain model and validation rules | High | TODO | - | - | - |
| 7 | API <-> engine bridge | .NET service calls Python engine over defined interface; API unit/integration tests | High | TODO | - | - | - |
| 8 | Qt dashboard UI (read-only) | List simulated endpoints + patch status table, polling/refresh against API, Qt CMake target | High | TODO | - | - | - |
| 9 | Schedule definition UI | Group/maintenance-window/rollout-stage editor forms wired to API | Medium | TODO | - | - | - |
| 10 | Control actions UI | Schedule, pause/resume, rollback buttons wired to API endpoints with confirmation + result feedback | Medium | TODO | - | - | - |
| 11 | End-to-end integration tests | Full GUI->API->engine flow exercised via automated integration suite | Medium | TODO | - | - | - |
| 12 | CI/CD pipeline | GitHub Actions: build all targets, run all test suites, artifact/release job | Medium | TODO | - | - | - |
| 13 | README + NinjaOne relevance story | Architecture docs, run/build instructions, screenshots, career relevance narrative | Low | TODO | - | - | - |
| 14 | Polish + hardening | Error handling, logging, packaging/installers, final review + release | Low | TODO | - | - | - |

Update the Status and Committed columns as phases complete. Commit tracker updates as
chore(...) commits.

## Dependencies / notes

- P1 must complete first; everything depends on the build skeleton.
- P2 (domain model) and P3 (Python engine) are independent of each other and can run in
  parallel after P1.
- P4 depends on P3; P5 and P6 depend on P2; P7 depends on P5 (API) and is scheduled after
  the engine is testable.
- P8–P10 (UI layers) depend on P7 (working API boundary) to consume live data.
- P11 depends on P8–P10; P12 depends on P4, P6, and P11 (all tests green). P13 and P14 can
  run late and are non-blocking.
