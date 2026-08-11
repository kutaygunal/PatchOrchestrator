// PatchOrchestrator — Sprint 37 (E6) log auto-refresh regression tests (Qt Test).
//
// T3 — Verifies E4 (audit log panel setLog/appendEntry), A1 (demo hub wiring)
// and B5/A3 (context binding) still work after the E6 auto-refresh changes.
// Runs offscreen (QT_QPA_PLATFORM=offscreen).

#include <QtTest/QtTest>

#include "ui/audit_log_panel.hpp"
#include "ui/demo_app_context.hpp"
#include "ui/demo_main_window.hpp"

namespace {

AuditLogEntry makeEntry(const QString &action, const QString &target,
                        const QString &timestamp, const QString &result)
{
    AuditLogEntry e;
    e.action = action;
    e.target = target;
    e.timestamp = timestamp;
    e.result = result;
    return e;
}

} // namespace

class E6RegressionTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_setLogRenders();
    void t2_appendEntryAppends();
    void t3_setLogReplacesPriorRows();
    void t4_hubContextBound();
};

void E6RegressionTests::t1_setLogRenders()
{
    AuditLogPanel panel;

    panel.setLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")),
    });

    QCOMPARE(panel.rowCount(), 2);
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("schedule"));
    QCOMPARE(panel.cellText(1, AuditLogPanel::ActionColumn),
             QStringLiteral("pause"));
}

void E6RegressionTests::t2_appendEntryAppends()
{
    AuditLogPanel panel;

    panel.appendEntry(makeEntry(QStringLiteral("resume"), QStringLiteral("sch-1"),
                                QStringLiteral("2025-01-01T09:10:00Z"),
                                QStringLiteral("resumed")));
    panel.appendEntry(makeEntry(QStringLiteral("rollback"), QStringLiteral("sch-2"),
                                QStringLiteral("2025-01-01T09:15:00Z"),
                                QStringLiteral("rolled_back")));

    QCOMPARE(panel.rowCount(), 2);
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("resume"));
    QCOMPARE(panel.cellText(1, AuditLogPanel::ActionColumn),
             QStringLiteral("rollback"));
}

void E6RegressionTests::t3_setLogReplacesPriorRows()
{
    AuditLogPanel panel;

    panel.setLog({makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                            QStringLiteral("t1"), QStringLiteral("paused"))});
    QCOMPARE(panel.rowCount(), 1);

    // A second setLog replaces the whole log (does not append).
    panel.setLog({makeEntry(QStringLiteral("resume"), QStringLiteral("sch-1"),
                            QStringLiteral("t2"), QStringLiteral("resumed"))});
    QCOMPARE(panel.rowCount(), 1);
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("resume"));
}

void E6RegressionTests::t4_hubContextBound()
{
    // A1: the demo hub embeds the audit log panel.
    DemoMainWindow window;
    QVERIFY(window.auditLog() != nullptr);
    QVERIFY(window.context() != nullptr);

    // A3/B5: the audit log panel is wired to the shared context, adopting the
    // active schedule id / API base URL, so its refresh uses the hub's state.
    AuditLogPanel *log = window.auditLog();
    QVERIFY(log->context() == window.context());

    window.context()->setScheduleId(QStringLiteral("sch-42"));
    QCOMPARE(log->scheduleId(), QStringLiteral("sch-42"));

    window.context()->setApiBaseUrl(QStringLiteral("http://example.test:9090"));
    QCOMPARE(log->apiBaseUrl(), QStringLiteral("http://example.test:9090"));

    // The panel still renders through the refresh path.
    log->applyFetchedLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-42"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
    });
    QCOMPARE(log->rowCount(), 1);
    QCOMPARE(log->cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("schedule"));

    // Stop the auto-refresh timer so no network is issued during teardown.
    log->stopRefresh();
}

QTEST_MAIN(E6RegressionTests)
#include "e6_regression_tests.moc"
