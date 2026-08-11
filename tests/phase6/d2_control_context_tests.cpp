// PatchOrchestrator — Sprint 26 (D2) failure rate control ↔ context tests (Qt Test).
//
// Covers the wiring between the failure-rate control and the shared
// DemoAppContext (A3):
//   * T1 — changing the control value updates the context's failure rate.
//   * T2 — the context reflects the new value.
//   * T3 — the control reads the initial value from the context.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QDoubleSpinBox>
#include <QSlider>

#include "ui/demo_app_context.hpp"
#include "ui/failure_rate_control.hpp"

class D2ControlContextTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_updatesContext();
    void t2_contextReflects();
    void t3_readsInitialValue();
};

void D2ControlContextTests::t1_updatesContext()
{
    DemoAppContext ctx;
    FailureRateControl control(&ctx);

    // Changing the spin box value updates the context's failure rate.
    control.spinBox()->setValue(0.17);
    QCOMPARE(ctx.failureRate(), 0.17);

    control.spinBox()->setValue(0.64);
    QCOMPARE(ctx.failureRate(), 0.64);

    // Changing the slider also updates the context's failure rate.
    control.slider()->setValue(90);
    QCOMPARE(ctx.failureRate(), 0.9);
}

void D2ControlContextTests::t2_contextReflects()
{
    DemoAppContext ctx;
    FailureRateControl control(&ctx);

    // The context reflects the new value after the control changes.
    control.spinBox()->setValue(0.09);
    QCOMPARE(ctx.failureRate(), 0.09);

    // A change made through the context propagates back into the control.
    ctx.setFailureRate(0.33);
    QCOMPARE(control.spinBox()->value(), 0.33);
    QCOMPARE(control.slider()->value(), 33);
}

void D2ControlContextTests::t3_readsInitialValue()
{
    DemoAppContext ctx;
    ctx.setFailureRate(0.77);

    // The control reads the initial value from the context on construction.
    FailureRateControl control(&ctx);
    QCOMPARE(control.spinBox()->value(), 0.77);
    QCOMPARE(control.slider()->value(), 77);
    QCOMPARE(control.failureRate(), 0.77);
}

QTEST_MAIN(D2ControlContextTests)
#include "d2_control_context_tests.moc"
