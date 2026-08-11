// PatchOrchestrator — Sprint 36 (E5) timestamp formatting helper.
//
// Converts an ISO-8601 timestamp string into a human-readable local-time
// string for display in the audit log panel. The conversion is self-contained
// and free of UI dependencies so it can be unit-tested on its own.

#ifndef PATCHORCHESTRATOR_UI_TIMESTAMP_FORMAT_HPP
#define PATCHORCHESTRATOR_UI_TIMESTAMP_FORMAT_HPP

#include <QString>

// Convert an ISO-8601 timestamp string (e.g. "2025-08-11T10:45:12Z" or with an
// explicit offset "+03:00") into a human-readable local-time string formatted
// as "yyyy-MM-dd HH:mm:ss". The timestamp is parsed as UTC when a trailing
// 'Z' or offset is present, then converted to the system's local time. If the
// input cannot be parsed (or is empty), the original string is returned
// unchanged so callers can display it safely.
QString formatTimestampLocal(const QString &isoTimestamp);

#endif // PATCHORCHESTRATOR_UI_TIMESTAMP_FORMAT_HPP
