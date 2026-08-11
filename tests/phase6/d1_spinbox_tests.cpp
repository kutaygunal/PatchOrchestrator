// PatchOrchestrator — Sprint 25 (D1) fleet size spin box tests (Qt Test).
//
// Covers the fleet-size spin box control itself:
//   * T1 — the spin box has a sensible range (minimum >= 1).
//   * T2 — setting the spin box value updates the fleet size.
//   * T3 — the value is stored in the shared context.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QSpinBox>

#include "ui/demo_app_context.hpp"
#include "ui/fleet_size_control.hpp"

class D1SpinBoxTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_sensibleRange();
    void t2_setsFleetSize();
    void t3_storedInContext();
};

void D1SpinBoxTests::t1_sensibleRange()
{
    DemoAppContext ctx;
    FleetSizeControl control(&ctx);

    // A fleet of zero endpoints makes no sense for a rollout.
    QVERIFY(control.spinBox()->minimum() >= 1);
    QVERIFY(control.spinBox()->maximum() > control.spinBox()->minimum());
}

void D1SpinBoxTests::t2_setsFleetSize()
{
    DemoAppContext ctx;
    FleetSizeControl control(&ctx);

    // Setting the spin box to a value updates the fleet size.
    control.spinBox()->setValue(25);
    QCOMPARE(control.fleetSize(), 25);
    QCOMPARE(ctx.fleetSize(), 25);

    control.spinBox()->setValue(3);
    QCOMPARE(control.fleetSize(), 3);
    QCOMPARE(ctx.fleetSize(), 3);
}

void D1SpinBoxTests::t3_storedInContext()
{
    DemoAppContext ctx;
    FleetSizeControl control(&ctx);

    // The value chosen in the spin box is stored in the shared context.
    control.spinBox()->setValue(42);
    QCOMPARE(ctx.fleetSize(), 42);

    // A second control bound to the same context sees the stored value.
    FleetSizeControl other(&ctx);
    QCOMPARE(other.spinBox()->value(), 42);
}

QTEST_MAIN(D1SpinBoxTests)
#include "d1_spinbox_tests.moc"
