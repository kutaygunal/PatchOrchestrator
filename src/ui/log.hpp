// PatchOrchestrator — small structured logger (timestamp + level + message).
//
// Replaces ad-hoc stderr/qDebug output in the Qt GUIs with a tiny, uniform
// structured format:
//
//     [2025-01-01T00:00:00.000Z] INFO  <message>
//     [2025-01-01T00:00:00.000Z] ERROR API request failed (foo)
//
// It is intentionally header-only and dependency-free so it can be used by
// every UI target without extra build wiring.

#ifndef PATCHORCHESTRATOR_UI_LOG_HPP
#define PATCHORCHESTRATOR_UI_LOG_HPP

#include <QDateTime>
#include <QString>
#include <QTextStream>

namespace patchorch::log {

enum class Level
{
    Debug,
    Info,
    Warn,
    Error
};

inline const char *levelName(Level level)
{
    switch (level) {
        case Level::Debug:
            return "DEBUG";
        case Level::Info:
            return "INFO";
        case Level::Warn:
            return "WARN";
        case Level::Error:
            return "ERROR";
    }
    return "INFO";
}

// Writes a single structured log record to stderr (so it never corrupts any
// stdout data channel) as `[ISO-8601] LEVEL message`.
inline void write(Level level, const QString &message)
{
    QTextStream out(stderr);
    out << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)
        << " [" << levelName(level) << "] " << message << '\n';
    out.flush();
}

} // namespace patchorch::log

#define PATCHORCH_LOG_DEBUG(msg) \
    ::patchorch::log::write(::patchorch::log::Level::Debug, (msg))
#define PATCHORCH_LOG_INFO(msg) \
    ::patchorch::log::write(::patchorch::log::Level::Info, (msg))
#define PATCHORCH_LOG_WARN(msg) \
    ::patchorch::log::write(::patchorch::log::Level::Warn, (msg))
#define PATCHORCH_LOG_ERROR(msg) \
    ::patchorch::log::write(::patchorch::log::Level::Error, (msg))

#endif // PATCHORCHESTRATOR_UI_LOG_HPP
