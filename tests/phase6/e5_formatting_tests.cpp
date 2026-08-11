// PatchOrchestrator — Sprint 36 (E5) timestamp formatting tests (Qt Test).
//
// T1 — Verifies ISO-8601 timestamps are formatted into a human-readable
// local-time string, that the format is readable (date + time), and that the
// local time is correct for each given timestamp. Also covers invalid/empty
// input, which must be handled gracefully. Runs offscreen
// (QT_QPA_PLATFORM=offscreen).

#include <QtTest/QtTest>
#include <QRegularExpression>

#include "ui/timestamp_format.hpp"

namespace {

// Expected local-time string computed independently with Qt's own ISO parse
// and local conversion, so the test is deterministic on any timezone.
QString expectedLocal(const QString &iso)
{
    return QDateTime::fromString(iso, Qt::ISODate)
        .toLocalTime()
        .toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

} // namespace

class E5FormattingTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_formatsToLocalTime();
    void t2_readableFormat();
    void t3_localTimeIsCorrect();
    void t4_offsetTimestamps();
    void t5_invalidInputPreserved();
    void t6_emptyInputPreserved();
};

void E5FormattingTests::t1_formatsToLocalTime()
{
    const QStringList inputs = {
        QStringLiteral("2025-08-11T10:45:12Z"),
        QStringLiteral("2025-01-01T00:00:00Z"),
        QStringLiteral("2025-12-31T23:59:59Z"),
    };

    for (const QString &iso : inputs) {
        const QString result = formatTimestampLocal(iso);
        // The formatted result must match the local-time conversion of the
        // same instant (i.e. it is a local-time display, not the raw string).
        QCOMPARE(result, expectedLocal(iso));
        // And it must differ from the raw ISO value (it was reformatted).
        QVERIFY2(result != iso, "timestamp was not reformatted");
    }
}

void E5FormattingTests::t2_readableFormat()
{
    // The output is a readable date + time, e.g. "2025-08-11 10:45:12".
    const QString result =
        formatTimestampLocal(QStringLiteral("2025-08-11T10:45:12Z"));

    QCOMPARE(result.size(), 19); // yyyy-MM-dd HH:mm:ss
    const QRegularExpression re(
        QStringLiteral("^\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}$"));
    QVERIFY2(re.match(result).hasMatch(),
             "formatted timestamp is not in readable yyyy-MM-dd HH:mm:ss form");
}

void E5FormattingTests::t3_localTimeIsCorrect()
{
    // Compare against the same instant computed by Qt using the system
    // timezone. Because formatTimestampLocal uses toLocalTime(), both must
    // agree for an arbitrary instant.
    const QString iso = QStringLiteral("2025-03-15T08:30:00Z");
    QCOMPARE(formatTimestampLocal(iso), expectedLocal(iso));
}

void E5FormattingTests::t4_offsetTimestamps()
{
    // An explicit offset (+03:00) must also be converted to local time.
    const QString iso = QStringLiteral("2025-08-11T10:45:12+03:00");
    QCOMPARE(formatTimestampLocal(iso), expectedLocal(iso));

    // Negative offset variant.
    const QString neg = QStringLiteral("2025-08-11T10:45:12-05:00");
    QCOMPARE(formatTimestampLocal(neg), expectedLocal(neg));
}

void E5FormattingTests::t5_invalidInputPreserved()
{
    // Unparseable input is returned unchanged (safe fallback).
    const QString bad = QStringLiteral("not-a-timestamp");
    QCOMPARE(formatTimestampLocal(bad), bad);

    const QString partiallyBad = QStringLiteral("2025-13-99T99:99:99Z");
    QCOMPARE(formatTimestampLocal(partiallyBad), partiallyBad);
}

void E5FormattingTests::t6_emptyInputPreserved()
{
    QCOMPARE(formatTimestampLocal(QString()), QString());
}

QTEST_MAIN(E5FormattingTests)
#include "e5_formatting_tests.moc"
