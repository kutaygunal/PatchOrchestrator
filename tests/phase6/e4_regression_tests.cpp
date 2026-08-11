// PatchOrchestrator — Sprint 35 (E4) audit log regression tests (Qt Test).
//
// T3 — Verifies the Sprint 35 (E4) audit log work did not break existing
// behaviour:
//   * T1 — the demo hub (A1) still constructs with all embedded panels and the
//     new Audit Log tab.
//   * T2 — the AuditLogPanel is wired into the hub and renders entries.
//   * T3 — the shared DemoAppContext (A3) is still bound across the panels.
//   * T4 — the E2/E3 action-log entry fields (action, target, timestamp,
//     result) render correctly through the hub's audit log panel.
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

} // namespace

class E4RegressionTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_demoHubConstructs();
    void t2_auditLogWired();
    void t3_sharedContextStillWorks();
    void t4_actionLogFieldsRender();
};

void E4RegressionTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void E4RegressionTests::t1_demoHubConstructs()
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

void E4RegressionTests::t2_auditLogWired()
{
    DemoMainWindow w;
    AuditLogPanel *panel = w.auditLog();
    QVERIFY(panel != nullptr);

    // The panel's table exists and can be driven directly.
    QVERIFY(panel->table() != nullptr);
    QCOMPARE(panel->rowCount(), 0);

    panel->appendEntry({QStringLiteral("pause"), QStringLiteral("sch-1"),
                        QStringLiteral("t1"), QStringLiteral("paused")});
    QCOMPARE(panel->rowCount(), 1);
}

void E4RegressionTests::t3_sharedContextStillWorks()
{
    DemoMainWindow w;

    // A3: a single shared context is still bound to the hub panels.
    DemoAppContext *ctx = w.context();
    QVERIFY(ctx != nullptr);
    QVERIFY(w.dashboard()->context() == ctx);
    QVERIFY(w.scheduleEditor()->context() == ctx);
    QVERIFY(w.controlPanel()->context() == ctx);

    // A3 defaults are unchanged.
    QCOMPARE(ctx->scheduleId(), QString());
    QCOMPARE(ctx->apiBaseUrl(), QStringLiteral("http://localhost:5000"));
    QCOMPARE(ctx->rolloutState(), QStringLiteral("idle"));
}

void E4RegressionTests::t4_actionLogFieldsRender()
{
    DemoMainWindow w;
    AuditLogPanel *panel = w.auditLog();
    QVERIFY(panel != nullptr);

    // E2/E3 ActionLogEntry fields (action, target, timestamp, result) render
    // in the hub's audit log panel.
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

QTEST_MAIN(E4RegressionTests)
#include "e4_regression_tests.moc"
