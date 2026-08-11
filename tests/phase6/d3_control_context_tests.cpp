// PatchOrchestrator — Sprint 27 (D3) seed control ↔ context tests (Qt Test).
//
// Covers the wiring between the seed control and the shared DemoAppContext (A3):
//   * T1 — changing the control value updates the context's seed.
//   * T2 — the context reflects the new value.
//   * T3 — the control reads the initial value from the context.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QSpinBox>

#include "ui/demo_app_context.hpp"
#include "ui/seed_control.hpp"

class D3ControlContextTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_updatesContext();
    void t2_contextReflects();
    void t3_readsInitialValue();
};

void D3ControlContextTests::t1_updatesContext()
{
    DemoAppContext ctx;
    SeedControl control(&ctx);

    // Changing the spin box value updates the context's seed.
    control.spinBox()->setValue(1234);
    QCOMPARE(ctx.seed(), 1234);

    control.spinBox()->setValue(5678);
    QCOMPARE(ctx.seed(), 5678);
}

void D3ControlContextTests::t2_contextReflects()
{
    DemoAppContext ctx;
    SeedControl control(&ctx);

    // The context reflects the new value after the control changes.
    control.spinBox()->setValue(42);
    QCOMPARE(ctx.seed(), 42);

    // A change made through the context propagates back into the control.
    ctx.setSeed(99);
    QCOMPARE(control.spinBox()->value(), 99);
}

void D3ControlContextTests::t3_readsInitialValue()
{
    DemoAppContext ctx;
    ctx.setSeed(777);

    // The control reads the initial value from the context on construction.
    SeedControl control(&ctx);
    QCOMPARE(control.spinBox()->value(), 777);
    QCOMPARE(control.seed(), 777);
}

QTEST_MAIN(D3ControlContextTests)
#include "d3_control_context_tests.moc"
