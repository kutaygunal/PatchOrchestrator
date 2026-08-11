// PatchOrchestrator — Sprint 36 (E5) audit log display tests (Qt Test).
//
// T2 — Verifies the AuditLogPanel displays timestamps in the human-readable
// local-time format rather than the raw ISO-8601 value, and that the formatted
// timestamps are clearly readable. Runs offscreen (QT_QPA_PLATFORM=offscreen).

#include <QtTest/QtTest>

#include "ui/audit_log_panel.hpp"

namespace {

// Independent computation of the expected local-time display for an ISO input,
// so the assertion is valid on any machine timezone.
QString expectedLocal(const QString &iso)
{
    return QDateTime::fromString(iso, Qt::ISODate)
        .toLocalTime()
        .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

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

class E5DisplayTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_displaysLocalTimeFormat();
    void t2_notRawIso();
    void t3_readableAcrossRows();
    void t4_appendDisplaysLocalTime();
};

void E5DisplayTests::t1_displaysLocalTimeFormat()
{
    AuditLogPanel panel;

    panel.setLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-08-11T10:45:12Z"),
                  QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-08-11T11:00:00Z"),
                  QStringLiteral("paused")),
    });

    QCOMPARE(panel.rowCount(), 2);
    QCOMPARE(panel.cellText(0, AuditLogPanel::TimestampColumn),
             expectedLocal(QStringLiteral("2025-08-11T10:45:12Z")));
    QCOMPARE(panel.cellText(1, AuditLogPanel::TimestampColumn),
             expectedLocal(QStringLiteral("2025-08-11T11:00:00Z")));
}

void E5DisplayTests::t2_notRawIso()
{
    AuditLogPanel panel;
    const QString iso = QStringLiteral("2025-08-11T10:45:12Z");

    panel.setLog({makeEntry(QStringLiteral("resume"), QStringLiteral("sch-2"),
                            iso, QStringLiteral("resumed"))});

    // The displayed timestamp must not be the raw ISO-8601 value.
    const QString shown = panel.cellText(0, AuditLogPanel::TimestampColumn);
    QVERIFY2(shown != iso, "panel displayed the raw ISO timestamp");
    // Other columns are unaffected.
    QCOMPARE(panel.cellText(0, AuditLogPanel::ActionColumn),
             QStringLiteral("resume"));
    QCOMPARE(panel.cellText(0, AuditLogPanel::TargetColumn),
             QStringLiteral("sch-2"));
    QCOMPARE(panel.cellText(0, AuditLogPanel::ResultColumn),
             QStringLiteral("resumed"));
}

void E5DisplayTests::t3_readableAcrossRows()
{
    AuditLogPanel panel;
    panel.setLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")),
        makeEntry(QStringLiteral("resume"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:10:00Z"), QStringLiteral("resumed")),
    });

    for (int row = 0; row < panel.rowCount(); ++row) {
        const QString shown = panel.cellText(row, AuditLogPanel::TimestampColumn);
        QCOMPARE(shown.size(), 19); // yyyy-MM-dd HH:mm:ss
    }
}

void E5DisplayTests::t4_appendDisplaysLocalTime()
{
    AuditLogPanel panel;
    const QString iso = QStringLiteral("2025-08-11T12:00:00Z");

    panel.appendEntry(makeEntry(QStringLiteral("rollback"), QStringLiteral("sch-9"),
                                iso, QStringLiteral("rolled_back")));

    QCOMPARE(panel.cellText(0, AuditLogPanel::TimestampColumn),
             expectedLocal(iso));
}

QTEST_MAIN(E5DisplayTests)
#include "e5_display_tests.moc"
