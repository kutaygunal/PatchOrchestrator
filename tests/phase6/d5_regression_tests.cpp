// PatchOrchestrator — Sprint 29 (D5) scenario selector regression tests (Qt Test).
//
// T3 — Verifies that the Sprint 29 (D5) scenario-selector work did not break
// the existing A3 (shared app state), B2/control-panel, and D1–D4 (fleet size,
// failure rate, seed, presets) behaviour:
//   * T1 — DemoAppContext (A3) still works correctly (set/get, defaults).
//   * T2 — the shared context is still bound across the demo hub panels.
//   * T3 — the control panel (B2/B7) still functions with the D1/D2/D3 controls
//     and now also exposes the D5 scenario selector.
//   * T4 — the D4 presets still load correctly and propagate to the bound
//     controls through the selector.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QComboBox>
#include <QDoubleSpinBox>
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
#include "ui/scenario_selector.hpp"
#include "ui/seed_control.hpp"

namespace {

const char kOrgName[] = "PatchOrchestratorTest";
const char kAppName[] = "PatchOrchestrator";

}  // namespace

class D5RegressionTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_contextWorks();
    void t2_sharedContext();
    void t3_controlPanelWorks();
    void t4_selectorPropagatesToControls();
};

void D5RegressionTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void D5RegressionTests::t1_contextWorks()
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

    // D1/D2/D3 defaults are unchanged and settable.
    QVERIFY(ctx.fleetSize() >= 1);
    ctx.setFleetSize(50);
    QCOMPARE(ctx.fleetSize(), 50);
    QVERIFY(ctx.failureRate() >= 0.0);
    QVERIFY(ctx.failureRate() <= 1.0);
    ctx.setFailureRate(0.4);
    QCOMPARE(ctx.failureRate(), 0.4);
    QCOMPARE(ctx.seed(), 0);
    ctx.setSeed(12345);
    QCOMPARE(ctx.seed(), 12345);
}

void D5RegressionTests::t2_sharedContext()
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

void D5RegressionTests::t3_controlPanelWorks()
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

    // The D5 scenario selector is now present and bound to the shared context.
    QVERIFY(w.controlPanel()->scenarioSelector() != nullptr);
    QVERIFY(w.controlPanel()->scenarioSelector()->context() == ctx);
    QVERIFY(w.controlPanel()->scenarioSelector()->comboBox() != nullptr);
    QCOMPARE(w.controlPanel()->scenarioSelector()->comboBox()->objectName(),
             QStringLiteral("scenarioComboBox"));

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

void D5RegressionTests::t4_selectorPropagatesToControls()
{
    DemoMainWindow w;
    DemoAppContext *ctx = w.context();
    ScenarioSelector *selector = w.controlPanel()->scenarioSelector();
    QVERIFY(selector != nullptr);

    // Manually set values that differ from every preset.
    w.controlPanel()->fleetSizeControl()->spinBox()->setValue(1);
    w.controlPanel()->failureRateControl()->spinBox()->setValue(0.02);
    w.controlPanel()->seedControl()->spinBox()->setValue(7);

    // Selecting a preset through the selector overrides the manual values and
    // populates the bound D1/D2/D3 controls with the preset values.
    const DemoScenario preset =
        scenario_presets::byName(QStringLiteral("high-failure fleet"));
    QVERIFY(!preset.name.isEmpty());

    const int index = selector->comboBox()->findText(preset.name);
    QVERIFY(index > 0);
    selector->comboBox()->setCurrentIndex(index);

    // The shared context reflects the preset.
    QCOMPARE(ctx->fleetSize(), preset.fleetSize);
    QCOMPARE(ctx->failureRate(), preset.failureRate);
    QCOMPARE(ctx->seed(), preset.seed);

    // The bound controls reflect the applied preset (not the manual values).
    QCOMPARE(w.controlPanel()->fleetSizeControl()->spinBox()->value(),
             preset.fleetSize);
    QCOMPARE(w.controlPanel()->failureRateControl()->spinBox()->value(),
             preset.failureRate);
    QCOMPARE(w.controlPanel()->seedControl()->spinBox()->value(), preset.seed);
}

QTEST_MAIN(D5RegressionTests)
#include "d5_regression_tests.moc"
