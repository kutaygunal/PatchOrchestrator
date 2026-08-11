// PatchOrchestrator — Sprint 30 (D6) invalid-config-blocked tests (Qt Test).
//
// T2 — Verifies that starting a rollout with invalid config is blocked by the
// control panel:
//   * an invalid fleet size blocks the Schedule action and no request is sent,
//   * an invalid failure rate blocks the Rollback action and no request is sent,
//   * the validation error label is populated when an action is blocked.
//
// A standalone (unbound) panel is used and the spin-box ranges are widened so
// invalid values (fleet size < 1, failure rate outside 0–1) persist in the
// controls. Because validation runs before the confirmation dialog and the
// network request, clicking a button with invalid config never sends anything,
// so the status text must not report a "Sending ..." request and the test never
// blocks on a confirmation dialog.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

#include "ui/control_panel.hpp"
#include "ui/failure_rate_control.hpp"
#include "ui/fleet_size_control.hpp"

class D6BlockedTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_scheduleBlockedOnInvalidFleetSize();
    void t2_rollbackBlockedOnInvalidFailureRate();
    void t3_scheduleBlockedOnInvalidFailureRate();
    void t4_blockedActionShowsValidationError();
};

void D6BlockedTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String("PatchOrchestratorTest"));
    QCoreApplication::setApplicationName(QLatin1String("PatchOrchestrator"));
}

void D6BlockedTests::t1_scheduleBlockedOnInvalidFleetSize()
{
    ControlPanelWindow w;
    w.setScheduleIdText(QStringLiteral("sch-1"));

    // Widen the fleet-size range so 0 is reachable (the default UI minimum is 1).
    auto *fleetSpin = w.fleetSizeControl()->spinBox();
    fleetSpin->setRange(0, 1000);
    fleetSpin->setValue(0);

    auto *button = w.findChild<QPushButton *>(QStringLiteral("scheduleButton"));
    QVERIFY(button != nullptr);
    button->click();

    // Blocked: no rollout request was sent (no confirmation dialog appeared).
    QVERIFY(!w.statusText().contains(QStringLiteral("Sending")));
    // An inline error is shown identifying the problem.
    QVERIFY(w.validationText().contains(QStringLiteral("fleet size")));
}

void D6BlockedTests::t2_rollbackBlockedOnInvalidFailureRate()
{
    ControlPanelWindow w;
    w.setScheduleIdText(QStringLiteral("sch-2"));

    // Widen the failure-rate range so 1.5 is reachable.
    auto *rateSpin = w.failureRateControl()->spinBox();
    rateSpin->setRange(-1.0, 2.0);
    rateSpin->setValue(1.5);

    auto *button = w.findChild<QPushButton *>(QStringLiteral("rollbackButton"));
    QVERIFY(button != nullptr);
    button->click();

    // Blocked: no rollback request was sent.
    QVERIFY(!w.statusText().contains(QStringLiteral("Sending")));
    QVERIFY(w.validationText().contains(QStringLiteral("failure rate")));
}

void D6BlockedTests::t3_scheduleBlockedOnInvalidFailureRate()
{
    ControlPanelWindow w;
    w.setScheduleIdText(QStringLiteral("sch-3"));

    // Widen the failure-rate range so a negative value is reachable.
    auto *rateSpin = w.failureRateControl()->spinBox();
    rateSpin->setRange(-1.0, 1.0);
    rateSpin->setValue(-0.5);

    auto *button = w.findChild<QPushButton *>(QStringLiteral("scheduleButton"));
    QVERIFY(button != nullptr);
    button->click();

    QVERIFY(!w.statusText().contains(QStringLiteral("Sending")));
    QVERIFY(w.validationText().contains(QStringLiteral("failure rate")));
}

void D6BlockedTests::t4_blockedActionShowsValidationError()
{
    ControlPanelWindow w;
    w.setScheduleIdText(QStringLiteral("sch-4"));

    // A valid config shows no validation error.
    QVERIFY(w.validationLabel() != nullptr);
    QVERIFY(w.validationText().isEmpty());

    // Introducing an invalid config populates the inline error label.
    auto *fleetSpin = w.fleetSizeControl()->spinBox();
    fleetSpin->setRange(0, 1000);
    fleetSpin->setValue(0);
    QVERIFY(!w.validationText().isEmpty());

    // The validation label is present as a named child (testable/inspectable).
    QVERIFY(w.findChild<QLabel *>(QStringLiteral("validationLabel")) != nullptr);
}

QTEST_MAIN(D6BlockedTests)
#include "d6_blocked_tests.moc"
