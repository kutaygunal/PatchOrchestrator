// PatchOrchestrator — Sprint 37 (E6) log auto-refresh tests (Qt Test).
//
// T1 — Verifies the AuditLogPanel refreshes and shows a new action promptly on
// an event: the panel updates through the refresh path (applyFetchedLog, the
// same code a successful fetch uses) without a manual setLog/appendEntry, and
// the new entry appears immediately. Also verifies the refresh timer can be
// started/stopped/configured. Runs offscreen (QT_QPA_PLATFORM=offscreen).

#include <QtTest/QtTest>

#include "ui/audit_log_panel.hpp"

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

class E6RefreshTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_panelRefreshesOnEvent();
    void t2_showsNewActionWithoutManualRefresh();
    void t3_promptRefreshOnEvent();
    void t4_refreshTimerControl();
};

void E6RefreshTests::t1_panelRefreshesOnEvent()
{
    AuditLogPanel panel;
    panel.setScheduleId(QStringLiteral("sch-1"));
    panel.setApiBaseUrl(QStringLiteral("http://localhost:5000"));

    panel.setLog({makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                            QStringLiteral("2025-01-01T09:00:00Z"),
                            QStringLiteral("ok"))});
    QCOMPARE(panel.rowCount(), 1);

    // A new action arrives (simulated stream event / fetch result). The panel
    // refreshes via the refresh path and now shows the new action.
    panel.applyFetchedLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")),
    });

    QCOMPARE(panel.rowCount(), 2);
    QCOMPARE(panel.cellText(1, AuditLogPanel::ActionColumn),
             QStringLiteral("pause"));
    QCOMPARE(panel.cellText(1, AuditLogPanel::ResultColumn),
             QStringLiteral("paused"));
}

void E6RefreshTests::t2_showsNewActionWithoutManualRefresh()
{
    AuditLogPanel panel;
    panel.setScheduleId(QStringLiteral("sch-2"));
    panel.setApiBaseUrl(QStringLiteral("http://localhost:5000"));

    // Initial state: no entries.
    QCOMPARE(panel.rowCount(), 0);

    // A single new action arrives through the refresh path only — no direct
    // setLog/appendEntry call by the caller.
    panel.applyFetchedLog({
        makeEntry(QStringLiteral("rollback"), QStringLiteral("sch-2"),
                  QStringLiteral("2025-02-02T08:00:00Z"), QStringLiteral("rolled_back")),
    });

    QVERIFY(panel.rowCount() == 1);
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("rollback"));
    QCOMPARE(panel.cellText(0, AuditLogPanel::TargetColumn),
             QStringLiteral("sch-2"));
    QCOMPARE(panel.cellText(0, AuditLogPanel::ResultColumn),
             QStringLiteral("rolled_back"));
}

void E6RefreshTests::t3_promptRefreshOnEvent()
{
    AuditLogPanel panel;
    panel.setScheduleId(QStringLiteral("sch-3"));

    // applyFetchedLog refreshes synchronously: the panel reflects the new log
    // immediately after the call returns, with no additional event processing.
    panel.applyFetchedLog({
        makeEntry(QStringLiteral("resume"), QStringLiteral("sch-3"),
                  QStringLiteral("2025-03-03T10:10:00Z"), QStringLiteral("resumed")),
    });

    QCOMPARE(panel.rowCount(), 1);
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("resume"));
    QCOMPARE(panel.cellText(0, AuditLogPanel::TimestampColumn).size(), 19);
}

void E6RefreshTests::t4_refreshTimerControl()
{
    AuditLogPanel panel;

    // Not refreshing by default.
    QVERIFY(!panel.isRefreshing());

    // Configure the interval.
    panel.setRefreshInterval(50);
    QCOMPARE(panel.refreshInterval(), 50);

    // Start refreshes and stop stops.
    panel.startRefresh();
    QVERIFY(panel.isRefreshing());
    panel.stopRefresh();
    QVERIFY(!panel.isRefreshing());
    panel.stopRefresh(); // idempotent
    QVERIFY(!panel.isRefreshing());
}

QTEST_MAIN(E6RefreshTests)
#include "e6_refresh_tests.moc"
