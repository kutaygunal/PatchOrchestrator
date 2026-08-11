// PatchOrchestrator — Sprint 28 (D4) scenario load tests (Qt Test).
//
// T2 — Verifies that loading a predefined scenario populates the fleet size,
// failure rate, and seed correctly and that the loaded values match the preset
// definition:
//   * applying each preset to a fresh DemoAppContext sets all three config
//     fields to the preset's values,
//   * the values in the context match the preset definition.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/demo_app_context.hpp"
#include "ui/demo_scenario.hpp"
#include "ui/scenario_presets.hpp"

class D4LoadTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_smallCleanLoads();
    void t2_largeLoads();
    void t3_highFailureLoads();
    void t4_loadsMatchPreset();
};

void D4LoadTests::t1_smallCleanLoads()
{
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("small clean fleet"));

    DemoAppContext ctx;
    ctx.applyScenario(preset);

    // The context now holds the small clean fleet's config.
    QCOMPARE(ctx.fleetSize(), preset.fleetSize);
    QCOMPARE(ctx.failureRate(), preset.failureRate);
    QCOMPARE(ctx.seed(), preset.seed);
}

void D4LoadTests::t2_largeLoads()
{
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("large fleet"));

    DemoAppContext ctx;
    ctx.applyScenario(preset);

    QCOMPARE(ctx.fleetSize(), preset.fleetSize);
    QCOMPARE(ctx.failureRate(), preset.failureRate);
    QCOMPARE(ctx.seed(), preset.seed);
}

void D4LoadTests::t3_highFailureLoads()
{
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("high-failure fleet"));

    DemoAppContext ctx;
    ctx.applyScenario(preset);

    QCOMPARE(ctx.fleetSize(), preset.fleetSize);
    QCOMPARE(ctx.failureRate(), preset.failureRate);
    QCOMPARE(ctx.seed(), preset.seed);
}

void D4LoadTests::t4_loadsMatchPreset()
{
    // Every preset, when applied to a fresh context, yields exactly the values
    // defined by the preset.
    for (const DemoScenario &preset : scenario_presets::all()) {
        DemoAppContext ctx;
        ctx.applyScenario(preset);

        QCOMPARE(ctx.fleetSize(), preset.fleetSize);
        QCOMPARE(ctx.failureRate(), preset.failureRate);
        QCOMPARE(ctx.seed(), preset.seed);
    }
}

QTEST_MAIN(D4LoadTests)
#include "d4_load_tests.moc"
