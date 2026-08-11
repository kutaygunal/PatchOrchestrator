// PatchOrchestrator — Sprint 30 (D6) inline-error display tests (Qt Test).
//
// T3 — Verifies that invalid config shows an inline error near the config
// controls, that the message identifies the offending field, and that the
// error clears once the value is corrected:
//   * an invalid fleet size shows "fleet size must be ≥ 1",
//   * an invalid failure rate shows "failure rate must be between 0 and 1",
//   * the error clears when the value is corrected.
//
// A standalone (unbound) panel is used and the spin-box ranges are widened so
// invalid values (fleet size < 1, failure rate outside 0–1) persist in the
// controls. Valid config shows no error.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QDoubleSpinBox>

#include "ui/control_panel.hpp"
#include "ui/failure_rate_control.hpp"
#include "ui/fleet_size_control.hpp"
#include "ui/seed_control.hpp"

class D6ErrorsTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_fleetSizeErrorShown();
    void t2_failureRateErrorShown();
    void t3_seedAlwaysValidNoError();
    void t4_errorClearsWhenCorrected();
};

void D6ErrorsTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String("PatchOrchestratorTest"));
    QCoreApplication::setApplicationName(QLatin1String("PatchOrchestrator"));
}

void D6ErrorsTests::t1_fleetSizeErrorShown()
{
    ControlPanelWindow w;

    // No error initially (valid default config).
    QVERIFY(w.validationText().isEmpty());

    // Invalid fleet size produces a message that identifies the problem.
    auto *fleetSpin = w.fleetSizeControl()->spinBox();
    fleetSpin->setRange(0, 1000);
    fleetSpin->setValue(0);
    QVERIFY(w.validationText().contains(QStringLiteral("fleet size")));
    QVERIFY(w.validationText().contains(QStringLiteral("≥ 1")));
}

void D6ErrorsTests::t2_failureRateErrorShown()
{
    ControlPanelWindow w;
    QVERIFY(w.validationText().isEmpty());

    // Out-of-range failure rates produce a message that identifies the problem.
    auto *rateSpin = w.failureRateControl()->spinBox();
    rateSpin->setRange(-1.0, 2.0);
    rateSpin->setValue(1.2);
    QVERIFY(w.validationText().contains(QStringLiteral("failure rate")));
    QVERIFY(w.validationText().contains(QStringLiteral("0 and 1")));

    rateSpin->setValue(-0.01);
    QVERIFY(w.validationText().contains(QStringLiteral("failure rate")));
}

void D6ErrorsTests::t3_seedAlwaysValidNoError()
{
    ControlPanelWindow w;

    // The seed is an integer and never invalid, so changing it never shows an
    // inline error on its own (with otherwise-valid config).
    w.seedControl()->spinBox()->setValue(12345);
    QVERIFY(w.validationText().isEmpty());
}

void D6ErrorsTests::t4_errorClearsWhenCorrected()
{
    ControlPanelWindow w;

    // Invalid fleet size shows an error.
    auto *fleetSpin = w.fleetSizeControl()->spinBox();
    fleetSpin->setRange(0, 1000);
    fleetSpin->setValue(0);
    QVERIFY(!w.validationText().isEmpty());

    // Correcting the value clears the error.
    fleetSpin->setValue(25);
    QVERIFY(w.validationText().isEmpty());

    // Correcting an invalid failure rate also clears the error.
    auto *rateSpin = w.failureRateControl()->spinBox();
    rateSpin->setRange(-1.0, 2.0);
    rateSpin->setValue(1.4);
    QVERIFY(!w.validationText().isEmpty());
    rateSpin->setValue(0.3);
    QVERIFY(w.validationText().isEmpty());
}

QTEST_MAIN(D6ErrorsTests)
#include "d6_errors_tests.moc"
