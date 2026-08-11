// PatchOrchestrator — Sprint 38 (E7) log export validity tests (Qt Test).
//
// T2 — Verifies the exported CSV has a header and correctly formatted rows,
// and that the exported content matches the audit log. Also verifies that
// fields containing commas or quotes are properly quoted/escaped so the file
// stays valid CSV. Runs offscreen (QT_QPA_PLATFORM=offscreen).

#include <QtTest/QtTest>

#include <QFile>
#include <QTextStream>
#include <QTemporaryDir>

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

class E7ValidTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_hasHeader();
    void t2_rowsFormatted();
    void t3_contentMatchesLog();
    void t4_specialCharactersEscaped();
};

void E7ValidTests::t1_hasHeader()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("log.csv"));

    AuditLogPanel panel;
    panel.setLog({makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                            QStringLiteral("t1"), QStringLiteral("paused"))});
    QVERIFY(panel.exportToFile(path));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    const QString header = in.readLine();

    // Header row lists the four fields in order.
    QCOMPARE(header, QStringLiteral("action,target,timestamp,result"));
}

void E7ValidTests::t2_rowsFormatted()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("log.csv"));

    AuditLogPanel panel;
    panel.setLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-3"),
                  QStringLiteral("2025-03-03T08:00:00Z"), QStringLiteral("ok")),
    });
    QVERIFY(panel.exportToFile(path));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    in.readLine(); // header
    const QString row = in.readLine();

    // A correctly formatted data row has exactly four comma-separated fields.
    const QStringList fields = row.split(QLatin1Char(','));
    QCOMPARE(fields.size(), 4);
    QCOMPARE(fields.value(0), QStringLiteral("schedule"));
    QCOMPARE(fields.value(1), QStringLiteral("sch-3"));
    QCOMPARE(fields.value(2), QStringLiteral("2025-03-03T08:00:00Z"));
    QCOMPARE(fields.value(3), QStringLiteral("ok"));
}

void E7ValidTests::t3_contentMatchesLog()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("log.csv"));

    const auto entries = QList<AuditLogEntry>{
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")),
        makeEntry(QStringLiteral("resume"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:10:00Z"), QStringLiteral("resumed")),
    };

    AuditLogPanel panel;
    panel.setLog(entries);
    QCOMPARE(panel.entries().size(), entries.size());
    QVERIFY(panel.exportToFile(path));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    const QStringList lines = in.readAll().split(QLatin1Char('\n'));

    // Header + each entry, in order, with the raw (unformatted) timestamps.
    QCOMPARE(lines.value(0), QStringLiteral("action,target,timestamp,result"));
    QCOMPARE(lines.value(1),
             QStringLiteral("schedule,sch-1,2025-01-01T09:00:00Z,ok"));
    QCOMPARE(lines.value(2),
             QStringLiteral("pause,sch-1,2025-01-01T09:05:00Z,paused"));
    QCOMPARE(lines.value(3),
             QStringLiteral("resume,sch-1,2025-01-01T09:10:00Z,resumed"));
}

void E7ValidTests::t4_specialCharactersEscaped()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("log.csv"));

    AuditLogPanel panel;
    panel.setLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch,1"),
                  QStringLiteral("t1"), QStringLiteral("he said \"ok\"")),
    });
    QVERIFY(panel.exportToFile(path));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    in.readLine(); // header
    const QString row = in.readLine();

    // Commas and quotes are escaped so the field stays a single CSV field.
    QVERIFY(row.contains(QStringLiteral("\"sch,1\"")));
    QVERIFY(row.contains(QStringLiteral("\"he said \"\"ok\"\"\"")));
}

QTEST_MAIN(E7ValidTests)
#include "e7_valid_tests.moc"
