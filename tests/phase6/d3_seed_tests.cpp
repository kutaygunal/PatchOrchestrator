// PatchOrchestrator — Sprint 27 (D3) seed control tests (Qt Test).
//
// Covers the seed control itself:
//   * T1 — the control accepts an integer seed within its range.
//   * T2 — setting the control to a value updates the seed.
//   * T3 — the value is stored in the shared context.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>
#include <QSpinBox>

#include "ui/demo_app_context.hpp"
#include "ui/seed_control.hpp"

class D3SeedTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_acceptsIntegerSeed();
    void t2_setsSeed();
    void t3_storedInContext();
};

void D3SeedTests::t1_acceptsIntegerSeed()
{
    DemoAppContext ctx;
    SeedControl control(&ctx);

    // The control is an integer spin box with a sensible documented range.
    QVERIFY(control.spinBox() != nullptr);
    QVERIFY(control.spinBox()->minimum() >= 0);
    QVERIFY(control.spinBox()->maximum() > control.spinBox()->minimum());

    // A default seed of 0 is the documented default.
    QCOMPARE(control.spinBox()->value(), 0);
}

void D3SeedTests::t2_setsSeed()
{
    DemoAppContext ctx;
    SeedControl control(&ctx);

    // Setting the spin box to a value updates the seed.
    control.spinBox()->setValue(250);
    QCOMPARE(control.seed(), 250);
    QCOMPARE(ctx.seed(), 250);

    control.spinBox()->setValue(8000);
    QCOMPARE(control.seed(), 8000);
    QCOMPARE(ctx.seed(), 8000);
}

void D3SeedTests::t3_storedInContext()
{
    DemoAppContext ctx;
    SeedControl control(&ctx);

    // The value chosen in the control is stored in the shared context.
    control.spinBox()->setValue(350);
    QCOMPARE(ctx.seed(), 350);

    // A second control bound to the same context sees the stored value.
    SeedControl other(&ctx);
    QCOMPARE(other.spinBox()->value(), 350);
    QCOMPARE(other.seed(), 350);
}

QTEST_MAIN(D3SeedTests)
#include "d3_seed_tests.moc"
