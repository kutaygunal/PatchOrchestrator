// PatchOrchestrator — Sprint 28 (D4) scenario preset data correctness (Qt Test).
//
// T1 — Verifies the predefined demo scenarios are defined and internally
// consistent:
//   * a preset exists for each scenario (small clean fleet, large fleet,
//     high-failure fleet),
//   * each preset defines a valid fleet size, failure rate (0.0–1.0), and seed,
//   * the values are internally consistent (small fleet is small, high-failure
//     fleet has a high failure rate, large fleet is large).
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/demo_scenario.hpp"
#include "ui/scenario_presets.hpp"

class D4PresetsTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_threePresetsExist();
    void t2_validFields();
    void t3_internallyConsistent();
    void t4_lookupByName();
};

void D4PresetsTests::t1_threePresetsExist()
{
    // All three named scenarios are defined.
    QVERIFY(scenario_presets::contains(QStringLiteral("small clean fleet")));
    QVERIFY(scenario_presets::contains(QStringLiteral("large fleet")));
    QVERIFY(scenario_presets::contains(QStringLiteral("high-failure fleet")));

    // The factory lists all three (and exactly the three).
    const auto presets = scenario_presets::all();
    QCOMPARE(presets.size(), 3);
}

void D4PresetsTests::t2_validFields()
{
    // Each preset has a valid fleet size (>0), failure rate (0.0–1.0), and seed.
    for (const DemoScenario &preset : scenario_presets::all()) {
        QVERIFY2(!preset.name.isEmpty(), "preset must have a name");
        QVERIFY2(preset.fleetSize >= 1, "fleet size must be at least 1");
        QVERIFY2(preset.failureRate >= 0.0, "failure rate must be >= 0.0");
        QVERIFY2(preset.failureRate <= 1.0, "failure rate must be <= 1.0");
        QVERIFY2(preset.seed >= 0, "seed must be a non-negative integer");
    }
}

void D4PresetsTests::t3_internallyConsistent()
{
    const DemoScenario small =
        scenario_presets::byName(QStringLiteral("small clean fleet"));
    const DemoScenario large =
        scenario_presets::byName(QStringLiteral("large fleet"));
    const DemoScenario highFailure =
        scenario_presets::byName(QStringLiteral("high-failure fleet"));

    // The large fleet is larger than the small fleet.
    QVERIFY(large.fleetSize > small.fleetSize);

    // The high-failure fleet has a high failure rate (well above the clean
    // fleets).
    QVERIFY(highFailure.failureRate > 0.4);
    QVERIFY(highFailure.failureRate > small.failureRate);
    QVERIFY(highFailure.failureRate > large.failureRate);

    // The clean fleets have a low failure rate.
    QVERIFY(small.failureRate <= 0.2);
    QVERIFY(large.failureRate <= 0.2);

    // The high-failure fleet is at least a moderate fleet size.
    QVERIFY(highFailure.fleetSize >= 20);
}

void D4PresetsTests::t4_lookupByName()
{
    // A known name returns the matching preset.
    const DemoScenario small =
        scenario_presets::byName(QStringLiteral("small clean fleet"));
    QCOMPARE(small.name, QStringLiteral("small clean fleet"));

    // An unknown name returns an empty scenario.
    const DemoScenario missing =
        scenario_presets::byName(QStringLiteral("does not exist"));
    QVERIFY(missing.name.isEmpty());
    QCOMPARE(scenario_presets::contains(QStringLiteral("does not exist")), false);
}

QTEST_MAIN(D4PresetsTests)
#include "d4_presets_tests.moc"
