// PatchOrchestrator — Sprint 29 (D5) scenario selector override tests (Qt Test).
//
// T2 — Verifies that selecting a preset overrides manually set values in the
// config controls:
//   * manually setting a fleet size, failure rate, and seed, then selecting a
//     preset, replaces those manual values with the preset's values,
//   * the controls reflect the preset values (not the manual ones).
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

class D5OverrideTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_overrideFleetSize();
    void t2_overrideFailureRate();
    void t3_overrideSeed();
    void t4_overrideAllValues();
};

void D5OverrideTests::t1_overrideFleetSize()
{
    DemoAppContext ctx;
    FleetSizeControl fleet(&ctx);
    FailureRateControl failure(&ctx);
    SeedControl seed(&ctx);
    ScenarioSelector selector(&ctx);

    // Manually set a fleet size that differs from the preset.
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("small clean fleet"));
    fleet.spinBox()->setValue(999);
    QCOMPARE(ctx.fleetSize(), 999);

    // Selecting the preset overrides the manual fleet size.
    const int index = selector.comboBox()->findText(preset.name);
    QVERIFY(index > 0);
    selector.comboBox()->setCurrentIndex(index);

    QCOMPARE(ctx.fleetSize(), preset.fleetSize);
    QCOMPARE(fleet.spinBox()->value(), preset.fleetSize);
}

void D5OverrideTests::t2_overrideFailureRate()
{
    DemoAppContext ctx;
    FleetSizeControl fleet(&ctx);
    FailureRateControl failure(&ctx);
    SeedControl seed(&ctx);
    ScenarioSelector selector(&ctx);

    // Manually set a failure rate that differs from the preset.
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("high-failure fleet"));
    failure.spinBox()->setValue(0.01);
    QCOMPARE(ctx.failureRate(), 0.01);

    const int index = selector.comboBox()->findText(preset.name);
    QVERIFY(index > 0);
    selector.comboBox()->setCurrentIndex(index);

    QCOMPARE(ctx.failureRate(), preset.failureRate);
    QCOMPARE(failure.spinBox()->value(), preset.failureRate);
}

void D5OverrideTests::t3_overrideSeed()
{
    DemoAppContext ctx;
    FleetSizeControl fleet(&ctx);
    FailureRateControl failure(&ctx);
    SeedControl seed(&ctx);
    ScenarioSelector selector(&ctx);

    // Manually set a seed that differs from the preset.
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("large fleet"));
    seed.spinBox()->setValue(42);
    QCOMPARE(ctx.seed(), 42);

    const int index = selector.comboBox()->findText(preset.name);
    QVERIFY(index > 0);
    selector.comboBox()->setCurrentIndex(index);

    QCOMPARE(ctx.seed(), preset.seed);
    QCOMPARE(seed.spinBox()->value(), preset.seed);
}

void D5OverrideTests::t4_overrideAllValues()
{
    DemoAppContext ctx;
    FleetSizeControl fleet(&ctx);
    FailureRateControl failure(&ctx);
    SeedControl seed(&ctx);
    ScenarioSelector selector(&ctx);

    // Manually set all three config values to values that differ from every
    // preset, then select a preset and confirm all manual values are overridden.
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("high-failure fleet"));
    QVERIFY(preset.fleetSize != 77);
    QVERIFY(preset.failureRate != 0.33);
    QVERIFY(preset.seed != 99);

    fleet.spinBox()->setValue(77);
    failure.spinBox()->setValue(0.33);
    seed.spinBox()->setValue(99);
    QCOMPARE(ctx.fleetSize(), 77);
    QCOMPARE(ctx.failureRate(), 0.33);
    QCOMPARE(ctx.seed(), 99);

    const int index = selector.comboBox()->findText(preset.name);
    QVERIFY(index > 0);
    selector.comboBox()->setCurrentIndex(index);

    // The preset values override the manual ones, in the context and controls.
    QCOMPARE(ctx.fleetSize(), preset.fleetSize);
    QCOMPARE(ctx.failureRate(), preset.failureRate);
    QCOMPARE(ctx.seed(), preset.seed);
    QCOMPARE(fleet.spinBox()->value(), preset.fleetSize);
    QCOMPARE(failure.spinBox()->value(), preset.failureRate);
    QCOMPARE(seed.spinBox()->value(), preset.seed);
}

QTEST_MAIN(D5OverrideTests)
#include "d5_override_tests.moc"
