// PatchOrchestrator — Sprint 25 (D1) regression tests (Qt Test).
//
// Verifies that the Sprint 25 (D1) fleet-size config work did not break the
// existing A3 (shared app state) and B2/control-panel behaviour:
//   * T1 — DemoAppContext (A3) still works correctly (set/get, defaults).
//   * T2 — the shared context is still bound across the demo hub panels.
//   * T3 — the control panel (B2/B7) still functions with the new control.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QSpinBox>

#include "ui/control_panel.hpp"
#include "ui/dashboard.hpp"
#include "ui/demo_app_context.hpp"
#include "ui/demo_main_window.hpp"
#include "ui/fleet_size_control.hpp"
#include "ui/schedule_editor.hpp"

namespace {

const char kOrgName[] = "PatchOrchestratorTest";
const char kAppName[] = "PatchOrchestrator";

}  // namespace

class D1RegressionTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_contextWorks();
    void t2_sharedContext();
    void t3_controlPanelWorks();
};

void D1RegressionTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void D1RegressionTests::t1_contextWorks()
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

    // The new D1 fleet size has a sensible default and is settable.
    QVERIFY(ctx.fleetSize() >= 1);
    ctx.setFleetSize(50);
    QCOMPARE(ctx.fleetSize(), 50);
}

void D1RegressionTests::t2_sharedContext()
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

void D1RegressionTests::t3_controlPanelWorks()
{
    DemoMainWindow w;
    DemoAppContext *ctx = w.context();

    // The control panel still exposes its fleet-size control, bound to the
    // shared context.
    QVERIFY(w.controlPanel()->fleetSizeControl() != nullptr);
    QVERIFY(w.controlPanel()->fleetSizeControl()->context() == ctx);

    // Changing the fleet size through the panel updates the shared context.
    w.controlPanel()->fleetSizeControl()->spinBox()->setValue(12);
    QCOMPARE(ctx->fleetSize(), 12);

    // The control panel's existing action-feedback path still works.
    w.controlPanel()->handleActionResult(QStringLiteral("running"),
                                         QStringLiteral("paused"));
    QCOMPARE(w.controlPanel()->lastKnownState(), QStringLiteral("paused"));
    QVERIFY(w.controlPanel()->confirmationText().contains(QStringLiteral("paused")));
}

QTEST_MAIN(D1RegressionTests)
#include "d1_regression_tests.moc"
