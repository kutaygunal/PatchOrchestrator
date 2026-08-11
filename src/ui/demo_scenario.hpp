// PatchOrchestrator — Sprint 28 (D4) scenario value type.
//
// A plain value type describing one predefined demo scenario. It carries the
// three config values the demo uses to build a fleet and drive a deterministic
// run: the fleet size, the per-endpoint failure rate (0.0–1.0), and the seed.
// This mirrors the shared DemoAppContext config fields so a preset can be
// applied directly to the context (see DemoAppContext::applyScenario).

#ifndef PATCHORCHESTRATOR_UI_DEMO_SCENARIO_HPP
#define PATCHORCHESTRATOR_UI_DEMO_SCENARIO_HPP

#include <QString>

struct DemoScenario
{
    QString name;
    int fleetSize = 0;
    double failureRate = 0.0;
    int seed = 0;
};

#endif // PATCHORCHESTRATOR_UI_DEMO_SCENARIO_HPP
