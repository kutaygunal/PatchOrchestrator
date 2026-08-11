// PatchOrchestrator — Sprint 37 (E6) log auto-refresh tests (Qt Test).
//
// T2 — Verifies that a sequence of action events produces a corresponding
// sequence of updates and that the final panel state matches the latest log.
// Each simulated fetch replaces the panel contents (setLog), so the panel
// always reflects the latest fetched log. Runs offscreen
// (QT_QPA_PLATFORM=offscreen).

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

class E6MultipleTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_sequenceOfUpdates();
    void t2_finalStateMatchesLatestLog();
};

void E6MultipleTests::t1_sequenceOfUpdates()
{
    AuditLogPanel panel;
    panel.setScheduleId(QStringLiteral("sch-1"));
    panel.setApiBaseUrl(QStringLiteral("http://localhost:5000"));

    // Event 1: one action.
    panel.applyFetchedLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
    });
    QCOMPARE(panel.rowCount(), 1);
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("schedule"));

    // Event 2: schedule + pause. The panel now reflects two actions.
    panel.applyFetchedLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")),
    });
    QCOMPARE(panel.rowCount(), 2);
    QCOMPARE(panel.cellText(1, AuditLogPanel::ActionColumn),
             QStringLiteral("pause"));

    // Event 3: schedule + pause + resume. The panel reflects three actions.
    panel.applyFetchedLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")),
        makeEntry(QStringLiteral("resume"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:10:00Z"), QStringLiteral("resumed")),
    });
    QCOMPARE(panel.rowCount(), 3);
    QCOMPARE(panel.cellText(2, AuditLogPanel::ActionColumn),
             QStringLiteral("resume"));
}

void E6MultipleTests::t2_finalStateMatchesLatestLog()
{
    AuditLogPanel panel;
    panel.setScheduleId(QStringLiteral("sch-1"));

    const QList<AuditLogEntry> latest = {
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")),
        makeEntry(QStringLiteral("resume"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:10:00Z"), QStringLiteral("resumed")),
        makeEntry(QStringLiteral("rollback"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:15:00Z"), QStringLiteral("rolled_back")),
    };

    // Feed several events; the final applyFetchedLog carries the latest log.
    panel.applyFetchedLog({latest.first()});
    panel.applyFetchedLog({latest.mid(0, 2)});
    panel.applyFetchedLog({latest.mid(0, 3)});
    panel.applyFetchedLog(latest);

    // The final panel state exactly matches the latest log: one row per entry,
    // with each column matching.
    QCOMPARE(panel.rowCount(), latest.size());
    for (int row = 0; row < latest.size(); ++row) {
        QCOMPARE(panel.cellText(row, AuditLogPanel::ActionColumn),
                 latest[row].action);
        QCOMPARE(panel.cellText(row, AuditLogPanel::TargetColumn),
                 latest[row].target);
        QCOMPARE(panel.cellText(row, AuditLogPanel::ResultColumn),
                 latest[row].result);
    }
}

QTEST_MAIN(E6MultipleTests)
#include "e6_multiple_tests.moc"
