// PatchOrchestrator — Sprint 30 (D6) config-validation regression tests (Qt Test).
//
// T4 — Verifies that existing behaviour is preserved alongside the new config
// validation:
//   * the D1/D2/D3 config controls still update the shared context,
//   * the control panel still exposes all control buttons and the B7 feedback
//     labels,
//   * the validation label is present and the config validator accepts valid
//     values through the panel.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

#include "ui/control_panel.hpp"
#include "ui/demo_app_context.hpp"
#include "ui/failure_rate_control.hpp"
#include "ui/fleet_size_control.hpp"
#include "ui/seed_control.hpp"

class D6RegressionTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_configControlsStillWork();
    void t2_panelButtonsAndFeedbackLabelsIntact();
    void t3_validationLabelPresent();
    void t4_validConfigNotBlocked();
};

void D6RegressionTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String("PatchOrchestratorTest"));
    QCoreApplication::setApplicationName(QLatin1String("PatchOrchestrator"));
}

void D6RegressionTests::t1_configControlsStillWork()
{
    // The D1/D2/D3 controls still write their values into the shared context.
    DemoAppContext ctx;
    FleetSizeControl fleet(&ctx);
    FailureRateControl failure(&ctx);
    SeedControl seed(&ctx);

    fleet.spinBox()->setValue(77);
    QCOMPARE(ctx.fleetSize(), 77);

    failure.spinBox()->setValue(0.35);
    QCOMPARE(ctx.failureRate(), 0.35);

    seed.spinBox()->setValue(99);
    QCOMPARE(ctx.seed(), 99);
}

void D6RegressionTests::t2_panelButtonsAndFeedbackLabelsIntact()
{
    // All control buttons are still present (B3 endpoint wiring).
    ControlPanelWindow w;
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("scheduleButton")) != nullptr);
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("pauseButton")) != nullptr);
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("resumeButton")) != nullptr);
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("rollbackButton")) != nullptr);
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("refreshButton")) != nullptr);

    // The B7 feedback labels are still present.
    QVERIFY(w.findChild<QLabel *>(QStringLiteral("diffLabel")) != nullptr);
    QVERIFY(w.findChild<QLabel *>(QStringLiteral("confirmationLabel")) != nullptr);

    // Shared-context wiring still works (B7): action results publish state.
    DemoAppContext ctx;
    w.setContext(&ctx);
    w.handleActionResult(QStringLiteral("running"), QStringLiteral("paused"));
    QCOMPARE(ctx.rolloutState(), QStringLiteral("paused"));
    QCOMPARE(w.lastKnownState(), QStringLiteral("paused"));
}

void D6RegressionTests::t3_validationLabelPresent()
{
    ControlPanelWindow w;
    QVERIFY(w.findChild<QLabel *>(QStringLiteral("validationLabel")) != nullptr);
    QVERIFY(w.validationLabel() != nullptr);

    // Valid default config: no inline error.
    QVERIFY(w.validationText().isEmpty());
}

void D6RegressionTests::t4_validConfigNotBlocked()
{
    DemoAppContext ctx;
    ControlPanelWindow w;
    w.setContext(&ctx);

    // A valid config passes validation through the panel.
    ctx.setFleetSize(10);
    ctx.setFailureRate(0.25);
    ctx.setSeed(42);
    QVERIFY(w.validateConfig());
    QVERIFY(w.validationText().isEmpty());
}

QTEST_MAIN(D6RegressionTests)
#include "d6_regression_tests.moc"
