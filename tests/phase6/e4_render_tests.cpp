// PatchOrchestrator — Sprint 35 (E4) audit log render tests (Qt Test).
//
// T1 — Verifies the AuditLogPanel renders log entries: one row per entry, and
// each row shows the action, target, timestamp, and result in the correct
// columns. Runs offscreen (QT_QPA_PLATFORM=offscreen).

#include <QtTest/QtTest>

#include "ui/audit_log_panel.hpp"

class E4RenderTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_rendersEntries();
    void t2_rendersAllFieldsPerColumn();
    void t3_setLogReplacesPriorRows();
    void t4_emptyLogHasNoRows();
};

void E4RenderTests::t1_rendersEntries()
{
    AuditLogPanel panel;

    QList<AuditLogEntry> entries;
    AuditLogEntry a;
    a.action = QStringLiteral("pause");
    a.target = QStringLiteral("sch-1");
    a.timestamp = QStringLiteral("2025-01-01T10:00:00Z");
    a.result = QStringLiteral("paused");
    entries.append(a);

    AuditLogEntry b;
    b.action = QStringLiteral("resume");
    b.target = QStringLiteral("sch-1");
    b.timestamp = QStringLiteral("2025-01-01T10:05:00Z");
    b.result = QStringLiteral("resumed");
    entries.append(b);

    AuditLogEntry c;
    c.action = QStringLiteral("rollback");
    c.target = QStringLiteral("sch-2");
    c.timestamp = QStringLiteral("2025-01-01T10:10:00Z");
    c.result = QStringLiteral("rolled_back");
    entries.append(c);

    panel.setLog(entries);

    // The number of displayed rows matches the log.
    QCOMPARE(panel.rowCount(), entries.size());
    QCOMPARE(panel.rowCount(), 3);
}

void E4RenderTests::t2_rendersAllFieldsPerColumn()
{
    AuditLogPanel panel;

    AuditLogEntry a;
    a.action = QStringLiteral("pause");
    a.target = QStringLiteral("sch-9");
    a.timestamp = QStringLiteral("2025-02-02T12:00:00Z");
    a.result = QStringLiteral("paused");

    AuditLogEntry b;
    b.action = QStringLiteral("schedule");
    b.target = QStringLiteral("sch-3");
    b.timestamp = QStringLiteral("2025-02-02T12:30:00Z");
    b.result = QStringLiteral("ok");

    panel.setLog({a, b});

    // Each entry shows its four fields in the correct columns.
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("pause"));
    QCOMPARE(panel.cellText(0, AuditLogPanel::TargetColumn),
             QStringLiteral("sch-9"));
    QCOMPARE(panel.cellText(0, AuditLogPanel::TimestampColumn),
             QStringLiteral("2025-02-02T12:00:00Z"));
    QCOMPARE(panel.cellText(0, AuditLogPanel::ResultColumn),
             QStringLiteral("paused"));

    QCOMPARE(panel.cellText(1, AuditLogPanel::ActionColumn),
             QStringLiteral("schedule"));
    QCOMPARE(panel.cellText(1, AuditLogPanel::TargetColumn),
             QStringLiteral("sch-3"));
    QCOMPARE(panel.cellText(1, AuditLogPanel::TimestampColumn),
             QStringLiteral("2025-02-02T12:30:00Z"));
    QCOMPARE(panel.cellText(1, AuditLogPanel::ResultColumn),
             QStringLiteral("ok"));
}

void E4RenderTests::t3_setLogReplacesPriorRows()
{
    AuditLogPanel panel;

    AuditLogEntry a;
    a.action = QStringLiteral("pause");
    a.target = QStringLiteral("sch-1");
    a.timestamp = QStringLiteral("t1");
    a.result = QStringLiteral("paused");
    panel.setLog({a});
    QCOMPARE(panel.rowCount(), 1);

    AuditLogEntry b;
    b.action = QStringLiteral("resume");
    b.target = QStringLiteral("sch-1");
    b.timestamp = QStringLiteral("t2");
    b.result = QStringLiteral("resumed");

    // A second setLog replaces the whole log (not appends).
    panel.setLog({b});
    QCOMPARE(panel.rowCount(), 1);
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("resume"));
}

void E4RenderTests::t4_emptyLogHasNoRows()
{
    AuditLogPanel panel;
    QCOMPARE(panel.rowCount(), 0);
    QVERIFY(panel.table() != nullptr);

    panel.setLog({});
    QCOMPARE(panel.rowCount(), 0);
}

QTEST_MAIN(E4RenderTests)
#include "e4_render_tests.moc"
