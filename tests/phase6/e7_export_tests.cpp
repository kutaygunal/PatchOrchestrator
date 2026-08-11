// PatchOrchestrator — Sprint 38 (E7) log export tests (Qt Test).
//
// T1 — Verifies that exporting the audit log produces a valid CSV file
// containing all log entries and their fields (action, target, timestamp,
// result). The log is populated through setLog, exported to a temp path, and
// read back to assert the file exists, has the expected structure, and holds
// every entry and field. Runs offscreen (QT_QPA_PLATFORM=offscreen).

#include <QtTest/QtTest>

#include <QFile>
#include <QPushButton>
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

class E7ExportTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_exportProducesFile();
    void t2_exportContainsAllEntries();
    void t3_exportContainsAllFields();
    void t4_exportButtonPresent();
};

void E7ExportTests::t1_exportProducesFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("log.csv"));

    AuditLogPanel panel;
    panel.setLog({makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                            QStringLiteral("2025-01-01T09:00:00Z"),
                            QStringLiteral("ok"))});

    QVERIFY(panel.exportToFile(path));

    // The export produced a real, non-empty file.
    QFile file(path);
    QVERIFY(file.exists());
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray data = file.readAll();
    QVERIFY(data.size() > 0);
}

void E7ExportTests::t2_exportContainsAllEntries()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("log.csv"));

    AuditLogPanel panel;
    panel.setLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:05:00Z"), QStringLiteral("paused")),
        makeEntry(QStringLiteral("resume"), QStringLiteral("sch-1"),
                  QStringLiteral("2025-01-01T09:10:00Z"), QStringLiteral("resumed")),
        makeEntry(QStringLiteral("rollback"), QStringLiteral("sch-2"),
                  QStringLiteral("2025-01-01T09:15:00Z"), QStringLiteral("rolled_back")),
    });
    const int expected = panel.rowCount();
    QCOMPARE(expected, 4);

    QVERIFY(panel.exportToFile(path));

    // Every row in the file (beyond the header) corresponds to one entry.
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    const QStringList lines = in.readAll().split(QLatin1Char('\n'));

    int dataRows = 0;
    for (const QString &line : lines) {
        if (!line.trimmed().isEmpty())
            ++dataRows;
    }
    // Header + one line per entry (QTextStream may not add a trailing empty
    // line for the final newline, so count non-empty lines).
    QCOMPARE(dataRows, expected + 1);
}

void E7ExportTests::t3_exportContainsAllFields()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("log.csv"));

    AuditLogPanel panel;
    panel.setLog({
        makeEntry(QStringLiteral("schedule"), QStringLiteral("sch-7"),
                  QStringLiteral("2025-02-02T12:00:00Z"), QStringLiteral("ok")),
        makeEntry(QStringLiteral("pause"), QStringLiteral("sch-7"),
                  QStringLiteral("2025-02-02T12:30:00Z"), QStringLiteral("paused")),
    });

    QVERIFY(panel.exportToFile(path));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    const QStringList lines = in.readAll().split(QLatin1Char('\n'));

    // Each data row must contain the four fields, comma-separated.
    const QString row0 = lines.value(1).trimmed();
    QVERIFY(row0.contains(QStringLiteral("schedule")));
    QVERIFY(row0.contains(QStringLiteral("sch-7")));
    QVERIFY(row0.contains(QStringLiteral("2025-02-02T12:00:00Z")));
    QVERIFY(row0.contains(QStringLiteral("ok")));

    const QString row1 = lines.value(2).trimmed();
    QVERIFY(row1.contains(QStringLiteral("pause")));
    QVERIFY(row1.contains(QStringLiteral("sch-7")));
    QVERIFY(row1.contains(QStringLiteral("2025-02-02T12:30:00Z")));
    QVERIFY(row1.contains(QStringLiteral("paused")));
}

void E7ExportTests::t4_exportButtonPresent()
{
    AuditLogPanel panel;
    QVERIFY(panel.exportButton() != nullptr);
    QCOMPARE(panel.exportButton()->objectName(), QStringLiteral("exportButton"));
    QCOMPARE(panel.exportButton()->text(), QStringLiteral("Export"));
}

QTEST_MAIN(E7ExportTests)
#include "e7_export_tests.moc"
