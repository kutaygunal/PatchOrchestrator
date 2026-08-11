// PatchOrchestrator — Phase 2 (P2) control-panel payload tests (Qt Test).
//
// P2 requires the control panel's Schedule button to send the fleet
// configuration in the POST /api/schedules body so the API (P1) can persist it
// as the shared source of truth. This suite verifies that the Schedule POST
// body includes fleetSize, failureRate, and seed, sourced from the existing
// config controls.
//
// Testable contract (engineer must add):
//   * ControlPanelWindow::QJsonObject schedulePayload() const  (public)
//     Returns the JSON body used by onSchedule() for the Schedule POST. It must
//     include: id, package, group_id, fleetSize, failureRate, seed. When a
//     DemoAppContext is bound, the fleet fields come from the context; otherwise
//     they fall back to the current values of the embedded config controls.
//     onSchedule() must send exactly this payload.
//
// Tests:
//   * T1 — payload carries fleetSize, failureRate, seed from the bound context.
//   * T2 — payload preserves id, package, group_id (regression on existing body).
//   * T3 — with no context bound, payload falls back to the config control values.
//   * T4 — regression: the Schedule button and config-validation still work.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QJsonObject>
#include <QJsonValue>
#include <QPushButton>

#include "ui/control_panel.hpp"
#include "ui/demo_app_context.hpp"

class P2ControlPayloadTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_fleetFieldsFromContext();
    void t2_regressionBodyPreserved();
    void t3_fallbackToControlsWithoutContext();
    void t4_regressionScheduleButton();
};

void P2ControlPayloadTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String("PatchOrchestratorTest"));
    QCoreApplication::setApplicationName(QLatin1String("PatchOrchestrator"));
}

void P2ControlPayloadTests::t1_fleetFieldsFromContext()
{
    ControlPanelWindow w;
    DemoAppContext ctx;
    ctx.setFleetSize(8);
    ctx.setFailureRate(0.35);
    ctx.setSeed(99);
    w.setContext(&ctx);
    w.setScheduleIdText(QStringLiteral("sch-p2"));

    const QJsonObject body = w.schedulePayload();

    QCOMPARE(body.value(QStringLiteral("fleetSize")).toInt(), 8);
    QCOMPARE(body.value(QStringLiteral("failureRate")).toDouble(), 0.35);
    QCOMPARE(body.value(QStringLiteral("seed")).toInt(), 99);
    QCOMPARE(body.value(QStringLiteral("id")).toString(), QStringLiteral("sch-p2"));
}

void P2ControlPayloadTests::t2_regressionBodyPreserved()
{
    ControlPanelWindow w;
    w.setScheduleIdText(QStringLiteral("sch-p2b"));

    const QJsonObject body = w.schedulePayload();

    // The existing id/package/group_id fields are still sent alongside the new
    // fleet fields (the API still requires id, and the control panel's prior
    // payload contract must not regress).
    QCOMPARE(body.value(QStringLiteral("id")).toString(), QStringLiteral("sch-p2b"));
    QCOMPARE(body.value(QStringLiteral("package")).toString(), QStringLiteral("pkg-v2"));
    QCOMPARE(body.value(QStringLiteral("group_id")).toString(), QStringLiteral("grp-1"));

    // The three fleet fields are present and of the correct JSON types.
    QVERIFY(body.contains(QStringLiteral("fleetSize")));
    QVERIFY(body.contains(QStringLiteral("failureRate")));
    QVERIFY(body.contains(QStringLiteral("seed")));
    QVERIFY(!body.value(QStringLiteral("fleetSize")).isNull());
    QVERIFY(!body.value(QStringLiteral("failureRate")).isNull());
    QVERIFY(!body.value(QStringLiteral("seed")).isNull());
}

void P2ControlPayloadTests::t3_fallbackToControlsWithoutContext()
{
    ControlPanelWindow w;

    // No context is bound: the payload must reflect the config controls' values.
    w.fleetSizeControl()->spinBox()->setValue(3);
    w.failureRateControl()->spinBox()->setValue(0.45);
    w.seedControl()->spinBox()->setValue(77);
    w.setScheduleIdText(QStringLiteral("sch-noctx"));

    const QJsonObject body = w.schedulePayload();

    QCOMPARE(body.value(QStringLiteral("fleetSize")).toInt(), 3);
    QCOMPARE(body.value(QStringLiteral("failureRate")).toDouble(), 0.45);
    QCOMPARE(body.value(QStringLiteral("seed")).toInt(), 77);
    QCOMPARE(body.value(QStringLiteral("id")).toString(), QStringLiteral("sch-noctx"));
}

void P2ControlPayloadTests::t4_regressionScheduleButton()
{
    ControlPanelWindow w;

    // The Schedule button is still present and the payload is built from the
    // same id/package/group_id + fleet fields regardless of validation state.
    QVERIFY(w.findChild<QPushButton *>(QStringLiteral("scheduleButton")) != nullptr);

    DemoAppContext ctx;
    ctx.setFleetSize(4);
    ctx.setFailureRate(0.2);
    ctx.setSeed(12);
    w.setContext(&ctx);
    w.setScheduleIdText(QStringLiteral("sch-p2d"));

    const QJsonObject body = w.schedulePayload();
    QCOMPARE(body.value(QStringLiteral("fleetSize")).toInt(), 4);
    QCOMPARE(body.value(QStringLiteral("failureRate")).toDouble(), 0.2);
    QCOMPARE(body.value(QStringLiteral("seed")).toInt(), 12);
}

QTEST_MAIN(P2ControlPayloadTests)
#include "p2_control_payload_tests.moc"
