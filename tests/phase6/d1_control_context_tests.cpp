// PatchOrchestrator — Sprint 25 (D1) fleet size control ↔ context tests (Qt Test).
//
// Covers the wiring between the fleet-size control and the shared
// DemoAppContext (A3):
//   * T1 — changing the spin box value updates the context's fleet size.
//   * T2 — the context reflects the new value.
//   * T3 — the control reads the initial value from the context.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QSpinBox>

#include "ui/demo_app_context.hpp"
#include "ui/fleet_size_control.hpp"

class D1ControlContextTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_updatesContext();
    void t2_contextReflects();
    void t3_readsInitialValue();
};

void D1ControlContextTests::t1_updatesContext()
{
    DemoAppContext ctx;
    FleetSizeControl control(&ctx);

    // Changing the spin box value updates the context's fleet size.
    control.spinBox()->setValue(17);
    QCOMPARE(ctx.fleetSize(), 17);

    control.spinBox()->setValue(64);
    QCOMPARE(ctx.fleetSize(), 64);
}

void D1ControlContextTests::t2_contextReflects()
{
    DemoAppContext ctx;
    FleetSizeControl control(&ctx);

    // The context reflects the new value after the control changes.
    control.spinBox()->setValue(9);
    QCOMPARE(ctx.fleetSize(), 9);

    // A change made through the context propagates back into the control.
    ctx.setFleetSize(33);
    QCOMPARE(control.spinBox()->value(), 33);
}

void D1ControlContextTests::t3_readsInitialValue()
{
    DemoAppContext ctx;
    ctx.setFleetSize(77);

    // The control reads the initial value from the context on construction.
    FleetSizeControl control(&ctx);
    QCOMPARE(control.spinBox()->value(), 77);
    QCOMPARE(control.fleetSize(), 77);
}

QTEST_MAIN(D1ControlContextTests)
#include "d1_control_context_tests.moc"
