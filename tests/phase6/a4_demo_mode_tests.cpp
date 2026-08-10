// PatchOrchestrator — Sprint 4 (A4) Demo Mode engine tests (Qt Test).
//
// Covers DemoModeController, the QObject that runs a guided scripted
// walkthrough (step list, narration, auto-advance, pause-on-step):
//   * T1 — step sequencing (start at first step, next/prev, complete at end).
//   * T2 — pause and stop (pause halts, resume continues, stop resets, no-op
//          when already paused).
//   * T3 — narration text updates (matches current step, updates on advance,
//          signal emitted with new narration).
//   * T4 — auto-advance (advances after delay, stops when paused/stopped,
//          completes at the last step).
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/demo_app_context.hpp"
#include "ui/demo_mode_controller.hpp"

namespace {

QVector<DemoStep> makeSteps()
{
    QVector<DemoStep> steps;
    steps.push_back({QStringLiteral("intro"), QStringLiteral("Welcome to the demo.")});
    steps.push_back({QStringLiteral("schedule"), QStringLiteral("Here is the schedule.")});
    steps.push_back({QStringLiteral("simulate"), QStringLiteral("Now we simulate a rollout.")});
    steps.push_back({QStringLiteral("rollback"), QStringLiteral("Finally, roll back.")});
    return steps;
}

}  // namespace

class A4DemoModeTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_stepSequencing();
    void t2_pauseAndStop();
    void t3_narrationUpdates();
    void t4_autoAdvance();
};

void A4DemoModeTests::t1_stepSequencing()
{
    DemoModeController c;
    c.setSteps(makeSteps());

    // Not running until started; starts at the first step.
    QVERIFY(!c.isRunning());
    QCOMPARE(c.currentIndex(), 0);
    QCOMPARE(c.currentStepId(), QStringLiteral("intro"));

    c.start();
    QVERIFY(c.isRunning());
    QCOMPARE(c.currentIndex(), 0);
    QCOMPARE(c.currentStepId(), QStringLiteral("intro"));

    // next() advances through the steps.
    c.next();
    QCOMPARE(c.currentIndex(), 1);
    QCOMPARE(c.currentStepId(), QStringLiteral("schedule"));
    c.next();
    QCOMPARE(c.currentIndex(), 2);
    QCOMPARE(c.currentStepId(), QStringLiteral("simulate"));

    // prev() returns to the previous step.
    c.prev();
    QCOMPARE(c.currentIndex(), 1);
    QCOMPARE(c.currentStepId(), QStringLiteral("schedule"));

    // prev() at the first step is a no-op.
    c.prev();
    c.prev();
    QCOMPARE(c.currentIndex(), 0);
    QCOMPARE(c.currentStepId(), QStringLiteral("intro"));

    // Advancing past the last step completes the walkthrough.
    c.next();
    c.next();
    c.next();
    QCOMPARE(c.currentIndex(), 3);
    QCOMPARE(c.currentStepId(), QStringLiteral("rollback"));
    c.next();
    QVERIFY(c.isComplete());
    QVERIFY(!c.isRunning());
    QCOMPARE(c.currentIndex(), 3);
    QCOMPARE(c.currentStepId(), QStringLiteral("rollback"));
}

void A4DemoModeTests::t2_pauseAndStop()
{
    DemoModeController c;
    c.setSteps(makeSteps());
    c.start();
    c.next();
    QCOMPARE(c.currentIndex(), 1);

    // pause() sets the paused state; the current step is retained.
    c.pause();
    QVERIFY(c.isPaused());
    QCOMPARE(c.currentIndex(), 1);

    // Pausing when already paused is a no-op (no crash, no spurious signal).
    int pausedSignals = 0;
    QObject::connect(&c, &DemoModeController::pausedChanged, &c,
                     [&pausedSignals](bool) { ++pausedSignals; });
    c.pause();
    QCOMPARE(pausedSignals, 0);

    // next() continues from the paused step (manual advance while paused).
    c.next();
    QCOMPARE(c.currentIndex(), 2);

    // resume() clears the paused state and continues.
    c.resume();
    QVERIFY(!c.isPaused());
    c.next();
    QCOMPARE(c.currentIndex(), 3);

    // stop() resets to a defined stopped state (back to start, not running).
    c.stop();
    QVERIFY(!c.isRunning());
    QVERIFY(!c.isPaused());
    QVERIFY(!c.isComplete());
    QCOMPARE(c.currentIndex(), 0);
    QCOMPARE(c.currentStepId(), QStringLiteral("intro"));
}

void A4DemoModeTests::t3_narrationUpdates()
{
    DemoModeController c;
    c.setSteps(makeSteps());

    // Narration matches the current step's text.
    QCOMPARE(c.narration(), QStringLiteral("Welcome to the demo."));

    // A narrationChanged signal is emitted with the new narration on advance.
    QString lastNarration;
    QObject::connect(&c, &DemoModeController::narrationChanged, &c,
                     [&lastNarration](const QString &n) { lastNarration = n; });

    c.start();
    QCOMPARE(c.narration(), QStringLiteral("Welcome to the demo."));
    QCOMPARE(lastNarration, QStringLiteral("Welcome to the demo."));

    c.next();
    QCOMPARE(c.narration(), QStringLiteral("Here is the schedule."));
    QCOMPARE(lastNarration, QStringLiteral("Here is the schedule."));

    c.next();
    QCOMPARE(c.narration(), QStringLiteral("Now we simulate a rollout."));
    QCOMPARE(lastNarration, QStringLiteral("Now we simulate a rollout."));

    // A stepChanged signal carries the new index and step id.
    int lastIndex = -1;
    QString lastId;
    QObject::connect(&c, &DemoModeController::stepChanged, &c,
                     [&lastIndex, &lastId](int i, const QString &id) {
                         lastIndex = i;
                         lastId = id;
                     });
    c.next();
    QCOMPARE(lastIndex, 3);
    QCOMPARE(lastId, QStringLiteral("rollback"));
    QCOMPARE(c.narration(), QStringLiteral("Finally, roll back."));
}

void A4DemoModeTests::t4_autoAdvance()
{
    DemoModeController c;
    c.setSteps(makeSteps());
    c.setAutoAdvanceInterval(50);
    c.start();
    QCOMPARE(c.currentIndex(), 0);

    // After the delay the controller advances automatically.
    QTest::qWait(120);
    QCOMPARE(c.currentIndex(), 1);

    // Auto-advance stops when paused.
    c.pause();
    QTest::qWait(120);
    QCOMPARE(c.currentIndex(), 1);

    // Resume continues auto-advancing.
    c.resume();
    QTest::qWait(120);
    QCOMPARE(c.currentIndex(), 2);

    // Auto-advance completes the walkthrough at the last step.
    QTest::qWait(400);
    QVERIFY(c.isComplete());
    QVERIFY(!c.isRunning());
    QCOMPARE(c.currentIndex(), 3);

    // A stopped controller does not auto-advance.
    DemoModeController c2;
    c2.setSteps(makeSteps());
    c2.setAutoAdvanceInterval(50);
    c2.start();
    c2.stop();
    QTest::qWait(120);
    QCOMPARE(c2.currentIndex(), 0);
    QVERIFY(!c2.isRunning());
}

QTEST_MAIN(A4DemoModeTests)
#include "a4_demo_mode_tests.moc"
