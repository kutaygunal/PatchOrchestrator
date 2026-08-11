// PatchOrchestrator — Sprint 28 (D4) regression tests (Qt Test).
//
// T3 — Verifies that the Sprint 28 (D4) scenario-preset work did not break the
// existing A3 (shared app state), B2/control-panel, and D1–D3 (fleet size,
// failure rate, seed) behaviour:
//   * T1 — DemoAppContext (A3) still works correctly (set/get, defaults).
//   * T2 — the shared context is still bound across the demo hub panels.
//   * T3 — the control panel (B2/B7) still functions with the D1/D2/D3
//     controls, and applying a preset through the context updates the bound
//     controls.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QSpinBox>

#include "ui/control_panel.hpp"
#include "ui/dashboard.hpp"
#include "ui/demo_app_context.hpp"
#include "ui/demo_main_window.hpp"
#include "ui/demo_scenario.hpp"
#include "ui/failure_rate_control.hpp"
#include "ui/fleet_size_control.hpp"
#include "ui/schedule_editor.hpp"
#include "ui/scenario_presets.hpp"
#include "ui/seed_control.hpp"

namespace {

const char kOrgName[] = "PatchOrchestratorTest";
const char kAppName[] = "PatchOrchestrator";

}  // namespace

class D4RegressionTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_contextWorks();
    void t2_sharedContext();
    void t3_controlPanelWorks();
    void t4_presetPropagatesToControls();
};

void D4RegressionTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void D4RegressionTests::t1_contextWorks()
{
    DemoAppContext ctx;

    // A3 defaults are unchanged.
    QCOMPARE(ctx.scheduleId(), QString());
    QCOMPARE(ctx.apiBaseUrl(), QStringLiteral("http://localhost:5000"));
    QCOMPARE(ctx.rolloutState(), QStringLiteral("idle"));

    // A3 set/get still works.
    ctx.setScheduleId(QStringLiteral("sch-1"));
    ctx.setApiBaseUrl(QStringLiteral("http://api.test"));
    ctx.setRolloutState(QStringLiteral("running"));
    QCOMPARE(ctx.scheduleId(), QStringLiteral("sch-1"));
    QCOMPARE(ctx.apiBaseUrl(), QStringLiteral("http://api.test"));
    QCOMPARE(ctx.rolloutState(), QStringLiteral("running"));

    // The D1 fleet size has a sensible default and is settable.
    QVERIFY(ctx.fleetSize() >= 1);
    ctx.setFleetSize(50);
    QCOMPARE(ctx.fleetSize(), 50);

    // The D2 failure rate has a sensible default and is settable.
    QVERIFY(ctx.failureRate() >= 0.0);
    QVERIFY(ctx.failureRate() <= 1.0);
    ctx.setFailureRate(0.4);
    QCOMPARE(ctx.failureRate(), 0.4);

    // The D3 seed has a documented default (0) and is settable.
    QCOMPARE(ctx.seed(), 0);
    ctx.setSeed(12345);
    QCOMPARE(ctx.seed(), 12345);
}

void D4RegressionTests::t2_sharedContext()
{
    DemoMainWindow w;

    // A single shared context is still bound to all three panels.
    DemoAppContext *ctx = w.context();
    QVERIFY(ctx != nullptr);
    QVERIFY(w.dashboard()->context() == ctx);
    QVERIFY(w.scheduleEditor()->context() == ctx);
    QVERIFY(w.controlPanel()->context() == ctx);

    // A change made through one panel is visible through the shared context.
    w.controlPanel()->setScheduleIdText(QStringLiteral("sch-shared"));
    QCOMPARE(ctx->scheduleId(), QStringLiteral("sch-shared"));
}

void D4RegressionTests::t3_controlPanelWorks()
{
    DemoMainWindow w;
    DemoAppContext *ctx = w.context();

    // The control panel still exposes its D1/D2/D3 controls bound to the
    // shared context.
    QVERIFY(w.controlPanel()->fleetSizeControl() != nullptr);
    QVERIFY(w.controlPanel()->fleetSizeControl()->context() == ctx);
    QVERIFY(w.controlPanel()->failureRateControl() != nullptr);
    QVERIFY(w.controlPanel()->failureRateControl()->context() == ctx);
    QVERIFY(w.controlPanel()->seedControl() != nullptr);
    QVERIFY(w.controlPanel()->seedControl()->context() == ctx);

    // Changes through the panel update the shared context.
    w.controlPanel()->fleetSizeControl()->spinBox()->setValue(12);
    QCOMPARE(ctx->fleetSize(), 12);
    w.controlPanel()->failureRateControl()->spinBox()->setValue(0.6);
    QCOMPARE(ctx->failureRate(), 0.6);
    w.controlPanel()->seedControl()->spinBox()->setValue(31415);
    QCOMPARE(ctx->seed(), 31415);

    // The control panel's existing action-feedback path still works.
    w.controlPanel()->handleActionResult(QStringLiteral("running"),
                                         QStringLiteral("paused"));
    QCOMPARE(w.controlPanel()->lastKnownState(), QStringLiteral("paused"));
    QVERIFY(w.controlPanel()->confirmationText().contains(QStringLiteral("paused")));
}

void D4RegressionTests::t4_presetPropagatesToControls()
{
    DemoMainWindow w;
    DemoAppContext *ctx = w.context();

    // Applying a predefined scenario through the shared context must update the
    // bound D1/D2/D3 controls to match the preset values.
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("high-failure fleet"));
    QVERIFY(!preset.name.isEmpty());

    ctx->applyScenario(preset);

    QCOMPARE(ctx->fleetSize(), preset.fleetSize);
    QCOMPARE(ctx->failureRate(), preset.failureRate);
    QCOMPARE(ctx->seed(), preset.seed);

    // The bound controls reflect the applied preset.
    QCOMPARE(w.controlPanel()->fleetSizeControl()->spinBox()->value(),
             preset.fleetSize);
    QCOMPARE(w.controlPanel()->failureRateControl()->spinBox()->value(),
             preset.failureRate);
    QCOMPARE(w.controlPanel()->seedControl()->spinBox()->value(), preset.seed);
}

QTEST_MAIN(D4RegressionTests)
#include "d4_regression_tests.moc"
