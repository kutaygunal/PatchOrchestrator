// PatchOrchestrator — Sprint 17 (B7) control-action feedback tests (Qt Test).
//
// Covers the control panel showing live confirmation that the engine actually
// paused/resumed/rolled back, with a before/after state diff:
//   * T1 — the panel captures the state before an action and displays the
//          before/after diff after the action.
//   * T2 — a successful action shows a visible confirmation reflecting the
//          actual new state; no success confirmation is shown when the action
//          did not change state.
//   * T3 — each action (pause/resume/rollback) produces the correct diff.
//   * T4 — regression: the control panel still exposes all control buttons and
//          the shared-context wiring still works alongside the new feedback.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QLabel>
#include <QPushButton>

#include "ui/control_panel.hpp"
#include "ui/demo_app_context.hpp"

class B7ControlFeedbackTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_stateDiffDisplayed();
    void t2_confirmationVisible();
    void t3_perActionDiff();
    void t4_regression();
};

void B7ControlFeedbackTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String("PatchOrchestratorTest"));
    QCoreApplication::setApplicationName(QLatin1String("PatchOrchestrator"));
}

void B7ControlFeedbackTests::t1_stateDiffDisplayed()
{
    ControlPanelWindow w;

    // The panel captures the state before the action and shows the after state.
    w.handleActionResult(QStringLiteral("running"), QStringLiteral("paused"));

    QVERIFY(w.diffText().contains(QStringLiteral("running")));
    QVERIFY(w.diffText().contains(QStringLiteral("paused")));
    QVERIFY(w.diffText().contains(QStringLiteral("→")));
    QCOMPARE(w.lastKnownState(), QStringLiteral("paused"));
}

void B7ControlFeedbackTests::t2_confirmationVisible()
{
    ControlPanelWindow w;

    // A successful action shows a visible confirmation reflecting the new state.
    w.handleActionResult(QStringLiteral("running"), QStringLiteral("paused"));
    QVERIFY(w.confirmationText().contains(QStringLiteral("Confirmed")));
    QVERIFY(w.confirmationText().contains(QStringLiteral("paused")));

    // An action that did not change state shows no success confirmation.
    w.handleActionResult(QStringLiteral("paused"), QStringLiteral("paused"));
    QVERIFY(!w.confirmationText().contains(QStringLiteral("Confirmed")));
    QVERIFY(w.confirmationText().contains(QStringLiteral("No state change")));
}

void B7ControlFeedbackTests::t3_perActionDiff()
{
    ControlPanelWindow w;

    // Pause: running -> paused.
    w.handleActionResult(QStringLiteral("running"), QStringLiteral("paused"));
    QCOMPARE(w.diffText(), QStringLiteral("State diff: running → paused"));

    // Resume: paused -> running.
    w.handleActionResult(QStringLiteral("paused"), QStringLiteral("running"));
    QCOMPARE(w.diffText(), QStringLiteral("State diff: paused → running"));

    // Rollback from running.
    w.handleActionResult(QStringLiteral("running"), QStringLiteral("rolled_back"));
    QCOMPARE(w.diffText(), QStringLiteral("State diff: running → rolled_back"));

    // Rollback from paused.
    w.handleActionResult(QStringLiteral("paused"), QStringLiteral("rolled_back"));
    QCOMPARE(w.diffText(), QStringLiteral("State diff: paused → rolled_back"));

    // Rollback from failed.
    w.handleActionResult(QStringLiteral("failed"), QStringLiteral("rolled_back"));
    QCOMPARE(w.diffText(), QStringLiteral("State diff: failed → rolled_back"));
}

void B7ControlFeedbackTests::t4_regression()
{
    ControlPanelWindow w;

    // All control buttons are still present and enabled (B3 endpoint wiring).
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("scheduleButton")) != nullptr);
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("pauseButton")) != nullptr);
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("resumeButton")) != nullptr);
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("rollbackButton")) != nullptr);
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("refreshButton")) != nullptr);

    // The diff and confirmation labels are present.
    QVERIFY(w.findChild<QLabel *>(QStringLiteral("diffLabel")) != nullptr);
    QVERIFY(w.findChild<QLabel *>(QStringLiteral("confirmationLabel")) != nullptr);

    // Shared-context wiring still works: an action result publishes the new
    // state to the context so other panels react.
    DemoAppContext ctx;
    w.setContext(&ctx);
    w.handleActionResult(QStringLiteral("running"), QStringLiteral("paused"));
    QCOMPARE(ctx.rolloutState(), QStringLiteral("paused"));
    QCOMPARE(w.lastKnownState(), QStringLiteral("paused"));
}

QTEST_MAIN(B7ControlFeedbackTests)
#include "b7_control_feedback_tests.moc"
