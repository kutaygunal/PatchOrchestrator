// PatchOrchestrator — Sprint 6 (A6) Demo Mode UI tests (Qt Test).
//
// Covers DemoModeBar, the widget that drives a DemoModeController:
//   * T1 — button actions trigger the controller (Start/Next/Prev/Stop).
//   * T2 — the current-step indicator updates as the controller advances.
//   * T3 — the narration text area displays the current step's narration.
//   * T4 — integration: a JSON script parsed by the S5 parser drives the bar
//          end-to-end with indicator + narration in sync.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

#include "ui/demo_mode_bar.hpp"
#include "ui/demo_mode_controller.hpp"
#include "ui/demo_script_parser.hpp"

namespace {

QVector<DemoStep> makeSteps()
{
    QVector<DemoStep> steps;
    steps.push_back({QStringLiteral("intro"), QStringLiteral("Welcome to the demo.")});
    steps.push_back({QStringLiteral("schedule"), QStringLiteral("Here is the schedule.")});
    steps.push_back({QStringLiteral("simulate"), QStringLiteral("Now we simulate a rollout.")});
    return steps;
}

}  // namespace

class A6DemoModeBarTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_buttonActions();
    void t2_stepIndicator();
    void t3_narration();
    void t4_integration();
};

void A6DemoModeBarTests::t1_buttonActions()
{
    DemoModeController controller;
    controller.setSteps(makeSteps());

    DemoModeBar bar;
    bar.setController(&controller);

    // Not running until Start is clicked.
    QVERIFY(!controller.isRunning());

    // Start drives the controller into the running state.
    QTest::mouseClick(bar.startButton(), Qt::LeftButton);
    QVERIFY(controller.isRunning());
    QCOMPARE(controller.currentIndex(), 0);

    // Next advances the controller.
    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QCOMPARE(controller.currentIndex(), 1);
    QCOMPARE(controller.currentStepId(), QStringLiteral("schedule"));

    // Prev returns to the previous step.
    QTest::mouseClick(bar.prevButton(), Qt::LeftButton);
    QCOMPARE(controller.currentIndex(), 0);
    QCOMPARE(controller.currentStepId(), QStringLiteral("intro"));

    // Stop resets the controller to a stopped state.
    QTest::mouseClick(bar.stopButton(), Qt::LeftButton);
    QVERIFY(!controller.isRunning());
    QCOMPARE(controller.currentIndex(), 0);
}

void A6DemoModeBarTests::t2_stepIndicator()
{
    DemoModeController controller;
    controller.setSteps(makeSteps());

    DemoModeBar bar;
    bar.setController(&controller);

    // Stopped before start.
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Stopped"));

    // Start shows "Step 1 of 3".
    QTest::mouseClick(bar.startButton(), Qt::LeftButton);
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Step 1 of 3"));

    // Advancing updates the indicator.
    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Step 2 of 3"));
    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Step 3 of 3"));

    // Going back updates the indicator.
    QTest::mouseClick(bar.prevButton(), Qt::LeftButton);
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Step 2 of 3"));

    // Stop returns the indicator to the stopped state.
    QTest::mouseClick(bar.stopButton(), Qt::LeftButton);
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Stopped"));
}

void A6DemoModeBarTests::t3_narration()
{
    DemoModeController controller;
    controller.setSteps(makeSteps());

    DemoModeBar bar;
    bar.setController(&controller);

    // The narration area is read-only.
    QVERIFY(bar.narrationArea()->isReadOnly());

    // Shows the current step's narration.
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Welcome to the demo."));

    // Advancing updates the narration text.
    QTest::mouseClick(bar.startButton(), Qt::LeftButton);
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Welcome to the demo."));
    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Here is the schedule."));
    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Now we simulate a rollout."));

    // Going back restores the previous narration.
    QTest::mouseClick(bar.prevButton(), Qt::LeftButton);
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Here is the schedule."));
}

void A6DemoModeBarTests::t4_integration()
{
    // A JSON demo script parsed by the S5 parser.
    const QString json = QStringLiteral(R"({
        "steps": [
            {"type": "load_scenario", "id": "intro", "narration": "Loading the scenario."},
            {"type": "schedule",     "id": "schedule", "narration": "Showing the schedule."},
            {"type": "simulate",     "id": "simulate", "narration": "Simulating a rollout."},
            {"type": "rollback",     "id": "rollback", "narration": "Rolling back."}
        ]
    })");

    const DemoScriptParser::Result parsed = DemoScriptParser::parse(json);
    QVERIFY(parsed.ok);
    QCOMPARE(parsed.steps.size(), 4);

    DemoModeController controller;
    controller.setSteps(DemoScriptParser::toControllerSteps(parsed.steps));

    DemoModeBar bar;
    bar.setController(&controller);

    // Drive the full walkthrough end-to-end via the bar controls.
    QTest::mouseClick(bar.startButton(), Qt::LeftButton);
    QVERIFY(controller.isRunning());
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Step 1 of 4"));
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Loading the scenario."));

    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Step 2 of 4"));
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Showing the schedule."));

    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Step 3 of 4"));
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Simulating a rollout."));

    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Step 4 of 4"));
    QCOMPARE(bar.narrationArea()->toPlainText(), QStringLiteral("Rolling back."));

    // Advancing past the last step completes the walkthrough.
    QTest::mouseClick(bar.nextButton(), Qt::LeftButton);
    QVERIFY(controller.isComplete());
    QVERIFY(!controller.isRunning());
    QCOMPARE(bar.stepIndicator()->text(), QStringLiteral("Stopped"));
}

QTEST_MAIN(A6DemoModeBarTests)
#include "a6_demo_mode_bar_tests.moc"
