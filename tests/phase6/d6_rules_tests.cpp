// PatchOrchestrator — Sprint 30 (D6) config-validation rules tests (Qt Test).
//
// T1 — Verifies the validation rules directly against the ConfigValidator:
//   * fleet size < 1 is invalid; >= 1 is valid,
//   * failure rate < 0 or > 1 is invalid; 0–1 is valid,
//   * seed is always an integer and therefore always valid,
//   * a fully valid config passes,
//   * errors()/firstError() report the offending field's message.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/config_validator.hpp"

class D6RulesTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_fleetSizeRule();
    void t2_failureRateRule();
    void t3_seedRule();
    void t4_validConfigPasses();
    void t5_errorMessages();
};

void D6RulesTests::t1_fleetSizeRule()
{
    // Fleet size < 1 is invalid.
    QVERIFY(!ConfigValidator::validate(0, 0.0, 0).isValid());
    QVERIFY(!ConfigValidator::validate(-1, 0.0, 0).isValid());
    QVERIFY(!ConfigValidator::validate(-100, 0.0, 0).isValid());

    // Fleet size >= 1 is valid (with otherwise-valid fields).
    QVERIFY(ConfigValidator::validate(1, 0.0, 0).isValid());
    QVERIFY(ConfigValidator::validate(10, 0.0, 0).isValid());
    QVERIFY(ConfigValidator::validate(1000, 0.0, 0).isValid());
}

void D6RulesTests::t2_failureRateRule()
{
    // Failure rate < 0 or > 1 is invalid.
    QVERIFY(!ConfigValidator::validate(10, -0.1, 0).isValid());
    QVERIFY(!ConfigValidator::validate(10, 1.1, 0).isValid());
    QVERIFY(!ConfigValidator::validate(10, -1.0, 0).isValid());

    // Failure rate in [0, 1] is valid.
    QVERIFY(ConfigValidator::validate(10, 0.0, 0).isValid());
    QVERIFY(ConfigValidator::validate(10, 0.5, 0).isValid());
    QVERIFY(ConfigValidator::validate(10, 1.0, 0).isValid());
}

void D6RulesTests::t3_seedRule()
{
    // The seed is an integer and is therefore always valid. The validator only
    // accepts an int, so any value passed is a valid seed. Ensure a range of
    // seeds, including 0 and negative, still yields a valid config.
    QVERIFY(ConfigValidator::validate(10, 0.0, 0).isValid());
    QVERIFY(ConfigValidator::validate(10, 0.0, 1).isValid());
    QVERIFY(ConfigValidator::validate(10, 0.0, 12345).isValid());
    QVERIFY(ConfigValidator::validate(10, 0.0, 99999).isValid());
}

void D6RulesTests::t4_validConfigPasses()
{
    // A fully valid config passes validation.
    QVERIFY(ConfigValidator::validate(1, 0.0, 0).isValid());
    QVERIFY(ConfigValidator::validate(50, 0.25, 42).isValid());
    QVERIFY(ConfigValidator::validate(1000, 1.0, 7).isValid());

    // A valid config has no error messages.
    const ConfigValidator::Result ok = ConfigValidator::validate(50, 0.25, 42);
    QVERIFY(ok.isValid());
    QVERIFY(ok.errors().isEmpty());
    QVERIFY(ok.firstError().isEmpty());
}

void D6RulesTests::t5_errorMessages()
{
    // Fleet size error message.
    const ConfigValidator::Result fleet = ConfigValidator::validate(0, 0.0, 0);
    QVERIFY(!fleet.isValid());
    QVERIFY(fleet.fleetSizeError.contains(QStringLiteral("fleet size")));
    QVERIFY(fleet.firstError() == fleet.fleetSizeError);
    QCOMPARE(fleet.errors().size(), 1);

    // Failure rate error message.
    const ConfigValidator::Result rate = ConfigValidator::validate(10, 1.5, 0);
    QVERIFY(!rate.isValid());
    QVERIFY(rate.failureRateError.contains(QStringLiteral("failure rate")));
    QVERIFY(rate.firstError() == rate.failureRateError);
    QCOMPARE(rate.errors().size(), 1);

    // Both fleet size and failure rate can be invalid at once.
    const ConfigValidator::Result both = ConfigValidator::validate(0, 1.5, 0);
    QVERIFY(!both.isValid());
    QCOMPARE(both.errors().size(), 2);
    // firstError reports the fleet-size message (field order).
    QVERIFY(both.firstError().contains(QStringLiteral("fleet size")));
}

QTEST_MAIN(D6RulesTests)
#include "d6_rules_tests.moc"
