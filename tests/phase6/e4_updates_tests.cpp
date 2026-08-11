// PatchOrchestrator — Sprint 35 (E4) audit log update tests (Qt Test).
//
// T2 — Verifies the AuditLogPanel updates with new entries: appending an entry
// renders a new row, the new entry appears, and the panel stays in sync with
// the log. Runs offscreen (QT_QPA_PLATFORM=offscreen).

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

class E4UpdateTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_appendAddsRow();
    void t2_newEntryAppears();
    void t3_staysInSyncAcrossManyAppends();
    void t4_appendAfterSetLog();
};

void E4UpdateTests::t1_appendAddsRow()
{
    AuditLogPanel panel;
    QCOMPARE(panel.rowCount(), 0);

    panel.appendEntry(makeEntry(QStringLiteral("pause"),
                                QStringLiteral("sch-1"),
                                QStringLiteral("t1"),
                                QStringLiteral("paused")));
    QCOMPARE(panel.rowCount(), 1);

    panel.appendEntry(makeEntry(QStringLiteral("resume"),
                                QStringLiteral("sch-1"),
                                QStringLiteral("t2"),
                                QStringLiteral("resumed")));
    QCOMPARE(panel.rowCount(), 2);
}

void E4UpdateTests::t2_newEntryAppears()
{
    AuditLogPanel panel;

    panel.appendEntry(makeEntry(QStringLiteral("pause"),
                                QStringLiteral("sch-1"),
                                QStringLiteral("t1"),
                                QStringLiteral("paused")));

    // Append a new entry and check it appears in the panel.
    panel.appendEntry(makeEntry(QStringLiteral("rollback"),
                                QStringLiteral("sch-2"),
                                QStringLiteral("t3"),
                                QStringLiteral("rolled_back")));

    const int last = panel.rowCount() - 1;
    QCOMPARE(panel.cellText(last, AuditLogPanel::ActionColumn),
             QStringLiteral("rollback"));
    QCOMPARE(panel.cellText(last, AuditLogPanel::TargetColumn),
             QStringLiteral("sch-2"));
    QCOMPARE(panel.cellText(last, AuditLogPanel::TimestampColumn),
             QStringLiteral("t3"));
    QCOMPARE(panel.cellText(last, AuditLogPanel::ResultColumn),
             QStringLiteral("rolled_back"));
}

void E4UpdateTests::t3_staysInSyncAcrossManyAppends()
{
    AuditLogPanel panel;

    // Append a larger sequence and verify the panel stays in sync with the
    // number of appends and the latest entry at each step.
    for (int i = 0; i < 20; ++i) {
        const QString action = QStringLiteral("action-%1").arg(i);
        panel.appendEntry(
            makeEntry(action, QStringLiteral("sch-1"),
                      QStringLiteral("t-%1").arg(i),
                      QStringLiteral("ok")));
        QCOMPARE(panel.rowCount(), i + 1);
        QCOMPARE(panel.cellText(i, AuditLogPanel::ActionColumn), action);
    }
}

void E4UpdateTests::t4_appendAfterSetLog()
{
    AuditLogPanel panel;

    panel.setLog({makeEntry(QStringLiteral("schedule"),
                            QStringLiteral("sch-1"),
                            QStringLiteral("t0"),
                            QStringLiteral("ok"))});
    QCOMPARE(panel.rowCount(), 1);

    // Appending after a setLog keeps the existing rows and adds one more.
    panel.appendEntry(makeEntry(QStringLiteral("pause"),
                                QStringLiteral("sch-1"),
                                QStringLiteral("t1"),
                                QStringLiteral("paused")));
    QCOMPARE(panel.rowCount(), 2);
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("schedule"));
    QCOMPARE(panel.cellText(1, AuditLogPanel::ActionColumn),
             QStringLiteral("pause"));
}

QTEST_MAIN(E4UpdateTests)
#include "e4_updates_tests.moc"
