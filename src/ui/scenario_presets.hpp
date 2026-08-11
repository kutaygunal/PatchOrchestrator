// PatchOrchestrator — Sprint 28 (D4) scenario presets.
//
// Exposes the three predefined demo scenarios used for demos and tests:
//   * small clean fleet — a small fleet with a low failure rate,
//   * large fleet       — a large fleet with a low failure rate,
//   * high-failure fleet — a moderate/large fleet with a high failure rate.
// Each preset carries a fleet size, a failure rate (0.0–1.0), and a seed so a
// run is deterministic. Presets are immutable and looked up by name; the
// factory also lists all of them.

#ifndef PATCHORCHESTRATOR_UI_SCENARIO_PRESETS_HPP
#define PATCHORCHESTRATOR_UI_SCENARIO_PRESETS_HPP

#include <QString>
#include <QVector>

#include "demo_scenario.hpp"

namespace scenario_presets {

// All predefined demo scenarios, in a stable order.
QVector<DemoScenario> all();

// Look up a preset by its (case-sensitive) name. Returns an empty DemoScenario
// (name empty, fields defaulted) when no preset matches.
DemoScenario byName(const QString &name);

// True when a preset with the given name exists.
bool contains(const QString &name);

}  // namespace scenario_presets

#endif // PATCHORCHESTRATOR_UI_SCENARIO_PRESETS_HPP
