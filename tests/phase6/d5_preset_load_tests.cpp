// PatchOrchestrator — Sprint 29 (D5) scenario selector preset-load tests (Qt Test).
//
// T1 — Verifies that selecting a preset in the scenario selector loads the
// preset's fleet size, failure rate, and seed into the bound config controls:
//   * the combo box lists every predefined scenario preset (plus a placeholder),
//   * selecting a preset propagates its values through the shared context to the
//     D1/D2/D3 controls,
//   * the controls reflect the preset values,
//   * all presets are selectable.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

#include "ui/demo_app_context.hpp"
#include "ui/demo_scenario.hpp"
#include "ui/failure_rate_control.hpp"
#include "ui/fleet_size_control.hpp"
#include "ui/scenario_presets.hpp"
#include "ui/scenario_selector.hpp"
#include "ui/seed_control.hpp"

class D5PresetLoadTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_listsAllPresets();
    void t2_selectingPresetLoadsControls();
    void t3_controlsReflectPreset();
    void t4_allPresetsSelectable();
};

void D5PresetLoadTests::t1_listsAllPresets()
{
    ScenarioSelector selector;

    // The combo box lists the placeholder plus one entry per predefined preset.
    const auto presets = scenario_presets::all();
    QCOMPARE(selector.presetCount(), presets.size());
    QCOMPARE(selector.comboBox()->count(), presets.size() + 1);

    // Every preset name is present in the combo box.
    for (const DemoScenario &preset : presets) {
        QVERIFY2(selector.comboBox()->findText(preset.name) >= 0,
                 "every preset must appear in the combo box");
    }
}

void D5PresetLoadTests::t2_selectingPresetLoadsControls()
{
    DemoAppContext ctx;
    FleetSizeControl fleet(&ctx);
    FailureRateControl failure(&ctx);
    SeedControl seed(&ctx);
    ScenarioSelector selector(&ctx);

    // Selecting the "high-failure fleet" preset loads its values into the
    // shared context (and thus the bound controls).
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("high-failure fleet"));
    QVERIFY(!preset.name.isEmpty());

    const int index = selector.comboBox()->findText(preset.name);
    QVERIFY(index > 0);
    selector.comboBox()->setCurrentIndex(index);

    QCOMPARE(ctx.fleetSize(), preset.fleetSize);
    QCOMPARE(ctx.failureRate(), preset.failureRate);
    QCOMPARE(ctx.seed(), preset.seed);
}

void D5PresetLoadTests::t3_controlsReflectPreset()
{
    DemoAppContext ctx;
    FleetSizeControl fleet(&ctx);
    FailureRateControl failure(&ctx);
    SeedControl seed(&ctx);
    ScenarioSelector selector(&ctx);

    // After selecting a preset, the D1/D2/D3 controls reflect the preset values.
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("large fleet"));
    QVERIFY(!preset.name.isEmpty());

    const int index = selector.comboBox()->findText(preset.name);
    QVERIFY(index > 0);
    selector.comboBox()->setCurrentIndex(index);

    QCOMPARE(fleet.spinBox()->value(), preset.fleetSize);
    QCOMPARE(failure.spinBox()->value(), preset.failureRate);
    QCOMPARE(seed.spinBox()->value(), preset.seed);
}

void D5PresetLoadTests::t4_allPresetsSelectable()
{
    DemoAppContext ctx;
    FleetSizeControl fleet(&ctx);
    FailureRateControl failure(&ctx);
    SeedControl seed(&ctx);
    ScenarioSelector selector(&ctx);

    // Every preset, when selected, loads its values into the controls.
    for (const DemoScenario &preset : scenario_presets::all()) {
        const int index = selector.comboBox()->findText(preset.name);
        QVERIFY(index > 0);
        selector.comboBox()->setCurrentIndex(index);

        QCOMPARE(ctx.fleetSize(), preset.fleetSize);
        QCOMPARE(ctx.failureRate(), preset.failureRate);
        QCOMPARE(ctx.seed(), preset.seed);
        QCOMPARE(fleet.spinBox()->value(), preset.fleetSize);
        QCOMPARE(failure.spinBox()->value(), preset.failureRate);
        QCOMPARE(seed.spinBox()->value(), preset.seed);
    }
}

QTEST_MAIN(D5PresetLoadTests)
#include "d5_preset_load_tests.moc"
