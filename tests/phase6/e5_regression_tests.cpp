// PatchOrchestrator — Sprint 36 (E5) audit log regression tests (Qt Test).
//
// T3 — Verifies the Sprint 36 (E5) timestamp-formatting work did not break
// existing behaviour:
//   * T1 — the demo hub (A1) still constructs with all embedded panels and the
//     Audit Log tab.
//   * T2 — the AuditLogPanel is still wired into the hub and renders entries.
//   * T3 — the shared DemoAppContext (A3) is still bound across the panels.
//   * T4 — E2/E3 action-log entries render correctly through the hub's audit
//     log panel, with timestamps formatted to local time (E5).
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/audit_log_panel.hpp"
#include "ui/control_panel.hpp"
#include "ui/dashboard.hpp"
#include "ui/demo_app_context.hpp"
#include "ui/demo_main_window.hpp"
#include "ui/schedule_editor.hpp"

namespace {

const char kOrgName[] = "PatchOrchestratorTest";
const char kAppName[] = "PatchOrchestrator";

QString expectedLocal(const QString &iso)
{
    return QDateTime::fromString(iso, Qt::ISODate)
        .toLocalTime()
        .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

} // namespace

class E5RegressionTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_demoHubConstructs();
    void t2_auditLogWired();
    void t3_sharedContextStillWorks();
    void t4_actionLogFieldsRender();
    void t5_timestampsFormattedLocalTime();
};

void E5RegressionTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void E5RegressionTests::t1_demoHubConstructs()
{
    DemoMainWindow w;

    // A1 hub still constructs with all embedded panels and the tab manager.
    QVERIFY(w.dashboard() != nullptr);
    QVERIFY(w.scheduleEditor() != nullptr);
    QVERIFY(w.controlPanel() != nullptr);
    QVERIFY(w.roadmap() != nullptr);
    QVERIFY(w.auditLog() != nullptr);
    QVERIFY(w.tabWidget() != nullptr);

    // The Audit Log tab is present.
    bool found = false;
    for (int i = 0; i < w.tabWidget()->count(); ++i) {
        if (w.tabWidget()->tabText(i) == QStringLiteral("Audit Log"))
            found = true;
    }
    QVERIFY(found);
}

void E5RegressionTests::t2_auditLogWired()
{
    DemoMainWindow w;
    AuditLogPanel *panel = w.auditLog();
    QVERIFY(panel != nullptr);

    QVERIFY(panel->table() != nullptr);
    QCOMPARE(panel->rowCount(), 0);

    panel->appendEntry({QStringLiteral("pause"), QStringLiteral("sch-1"),
                        QStringLiteral("t1"), QStringLiteral("paused")});
    QCOMPARE(panel->rowCount(), 1);
}

void E5RegressionTests::t3_sharedContextStillWorks()
{
    DemoMainWindow w;

    DemoAppContext *ctx = w.context();
    QVERIFY(ctx != nullptr);
    QVERIFY(w.dashboard()->context() == ctx);
    QVERIFY(w.scheduleEditor()->context() == ctx);
    QVERIFY(w.controlPanel()->context() == ctx);

    QCOMPARE(ctx->scheduleId(), QString());
    QCOMPARE(ctx->apiBaseUrl(), QStringLiteral("http://localhost:5000"));
    QCOMPARE(ctx->rolloutState(), QStringLiteral("idle"));
}

void E5RegressionTests::t4_actionLogFieldsRender()
{
    DemoMainWindow w;
    AuditLogPanel *panel = w.auditLog();
    QVERIFY(panel != nullptr);

    panel->setLog({
        {QStringLiteral("schedule"), QStringLiteral("sch-1"),
         QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")},
        {QStringLiteral("pause"), QStringLiteral("sch-1"),
         QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")},
        {QStringLiteral("resume"), QStringLiteral("sch-1"),
         QStringLiteral("2025-01-01T09:10:00Z"), QStringLiteral("resumed")},
    });

    QCOMPARE(panel->rowCount(), 3);
    QCOMPARE(panel->cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("schedule"));
    QCOMPARE(panel->cellText(0, AuditLogPanel::TargetColumn),
             QStringLiteral("sch-1"));
    QCOMPARE(panel->cellText(1, AuditLogPanel::ResultColumn),
             QStringLiteral("paused"));
    QCOMPARE(panel->cellText(2, AuditLogPanel::ActionColumn),
             QStringLiteral("resume"));
}

void E5RegressionTests::t5_timestampsFormattedLocalTime()
{
    DemoMainWindow w;
    AuditLogPanel *panel = w.auditLog();
    QVERIFY(panel != nullptr);

    const QString iso = QStringLiteral("2025-01-01T09:00:00Z");
    panel->setLog({{QStringLiteral("schedule"), QStringLiteral("sch-1"),
                    iso, QStringLiteral("ok")}});

    // E5: the timestamp column shows the formatted local-time string.
    const QString shown = panel->cellText(0, AuditLogPanel::TimestampColumn);
    QCOMPARE(shown, expectedLocal(iso));
    QVERIFY2(shown != iso, "timestamp column still shows raw ISO value");
}

QTEST_MAIN(E5RegressionTests)
#include "e5_regression_tests.moc"
