// PatchOrchestrator — Sprint 36 (E5) timestamp formatting helper.

#include "timestamp_format.hpp"

#include <QDateTime>

QString formatTimestampLocal(const QString &isoTimestamp)
{
    if (isoTimestamp.isEmpty())
        return isoTimestamp;

    // Parse the ISO-8601 timestamp. A trailing 'Z' or explicit offset yields a
    // QDateTime whose time spec carries the correct UTC/offset information.
    const QDateTime utc = QDateTime::fromString(isoTimestamp, Qt::ISODate);
    if (!utc.isValid())
        return isoTimestamp;

    // Convert to the system's local time and format for display.
    return utc.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}
