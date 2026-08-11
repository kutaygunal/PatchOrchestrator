// PatchOrchestrator — Sprint 26 (D2) failure rate control tests (Qt Test).
//
// Covers the failure-rate control itself:
//   * T1 — the control has a range of 0.0–1.0.
//   * T2 — setting the control to a value updates the failure rate.
//   * T3 — the value is stored in the shared context.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QDoubleSpinBox>
#include <QSlider>

#include "ui/demo_app_context.hpp"
#include "ui/failure_rate_control.hpp"

class D2RangeTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_range();
    void t2_setsFailureRate();
    void t3_storedInContext();
};

void D2RangeTests::t1_range()
{
    DemoAppContext ctx;
    FailureRateControl control(&ctx);

    // The control has a range of 0.0–1.0.
    QCOMPARE(control.spinBox()->minimum(), 0.0);
    QCOMPARE(control.spinBox()->maximum(), 1.0);
    QVERIFY(control.spinBox()->maximum() > control.spinBox()->minimum());

    // The slider mirrors the same 0–100% range.
    QCOMPARE(control.slider()->minimum(), 0);
    QCOMPARE(control.slider()->maximum(), 100);
}

void D2RangeTests::t2_setsFailureRate()
{
    DemoAppContext ctx;
    FailureRateControl control(&ctx);

    // Setting the spin box to a value updates the failure rate.
    control.spinBox()->setValue(0.25);
    QCOMPARE(control.failureRate(), 0.25);
    QCOMPARE(ctx.failureRate(), 0.25);

    control.spinBox()->setValue(0.8);
    QCOMPARE(control.failureRate(), 0.8);
    QCOMPARE(ctx.failureRate(), 0.8);

    // Setting the slider also updates the failure rate.
    control.slider()->setValue(50);
    QCOMPARE(control.failureRate(), 0.5);
    QCOMPARE(ctx.failureRate(), 0.5);
}

void D2RangeTests::t3_storedInContext()
{
    DemoAppContext ctx;
    FailureRateControl control(&ctx);

    // The value chosen in the control is stored in the shared context.
    control.spinBox()->setValue(0.35);
    QCOMPARE(ctx.failureRate(), 0.35);

    // A second control bound to the same context sees the stored value.
    FailureRateControl other(&ctx);
    QCOMPARE(other.spinBox()->value(), 0.35);
    QCOMPARE(other.failureRate(), 0.35);
}

QTEST_MAIN(D2RangeTests)
#include "d2_range_tests.moc"
