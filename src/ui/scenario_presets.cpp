// PatchOrchestrator — Sprint 28 (D4) scenario presets implementation.

#include "scenario_presets.hpp"

namespace {

// The three predefined demo scenarios. Values are chosen to be internally
// consistent with each scenario's intent:
//   * small clean fleet      — small, low failure rate, deterministic seed.
//   * large fleet            — large, low failure rate, deterministic seed.
//   * high-failure fleet     — moderate fleet, high failure rate, seed.
const DemoScenario kSmallCleanFleet = {
    QStringLiteral("small clean fleet"),
    /* fleetSize   */ 10,
    /* failureRate */ 0.05,
    /* seed        */ 1001,
};

const DemoScenario kLargeFleet = {
    QStringLiteral("large fleet"),
    /* fleetSize   */ 100,
    /* failureRate */ 0.10,
    /* seed        */ 2002,
};

const DemoScenario kHighFailureFleet = {
    QStringLiteral("high-failure fleet"),
    /* fleetSize   */ 50,
    /* failureRate */ 0.60,
    /* seed        */ 3003,
};

}  // namespace

namespace scenario_presets {

QVector<DemoScenario> all()
{
    return {kSmallCleanFleet, kLargeFleet, kHighFailureFleet};
}

DemoScenario byName(const QString &name)
{
    for (const DemoScenario &preset : all()) {
        if (preset.name == name)
            return preset;
    }
    return DemoScenario{};
}

bool contains(const QString &name)
{
    for (const DemoScenario &preset : all()) {
        if (preset.name == name)
            return true;
    }
    return false;
}

}  // namespace scenario_presets
