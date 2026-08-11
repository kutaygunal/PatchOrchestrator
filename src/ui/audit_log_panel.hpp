// PatchOrchestrator — Sprint 35 (E4) / Sprint 37 (E6) audit log panel.
//
// A QTableWidget panel in the demo hub that displays the live operator action
// log. Each entry shows the action, target, timestamp, and result in one row.
// The panel can replace the whole log (setLog) or append new entries
// (appendEntry), so it stays in sync with the recorded action log.
//
// Sprint 37 (E6) adds auto-refresh: a QTimer-driven poll fetches the action log
// (GET /api/schedules/{id}/actions) and refreshes the panel via setLog, so it
// updates in real time as actions occur. The refresh can be wired to the shared
// DemoAppContext (A3) or driven explicitly via setScheduleId/setApiBaseUrl, and
// tests can inject simulated fetch results deterministically offscreen.

#ifndef PATCHORCHESTRATOR_UI_AUDIT_LOG_PANEL_HPP
#define PATCHORCHESTRATOR_UI_AUDIT_LOG_PANEL_HPP

#include <QList>
#include <QNetworkAccessManager>
#include <QString>
#include <QTimer>
#include <QWidget>

class QNetworkReply;
class QTableWidget;
class DemoAppContext;

// A single operator action log entry (mirrors the .NET E1/E2 ActionLogEntry
// fields: action, target, timestamp, result).
struct AuditLogEntry
{
    QString action;
    QString target;
    QString timestamp;
    QString result;
};

class AuditLogPanel : public QWidget
{
    Q_OBJECT

public:
    // Display columns, in order.
    enum Column {
        ActionColumn = 0,
        TargetColumn,
        TimestampColumn,
        ResultColumn,
        ColumnCount
    };

    explicit AuditLogPanel(QWidget *parent = nullptr);

    // Replace the whole log with the given entries, clearing any prior rows.
    void setLog(const QList<AuditLogEntry> &entries);

    // Append a single new entry, rendering one more row.
    void appendEntry(const AuditLogEntry &entry);

    // Test accessors.
    QTableWidget *table() const { return m_table; }
    int rowCount() const;
    QString cellText(int row, int column) const;

    // --- Sprint 37 (E6): log auto-refresh ----------------------------------
    //
    // The panel can be wired to the shared app context (A3) and/or driven
    // directly with explicit schedule id / API base URL. A QTimer-driven poll
    // fetches the action log and refreshes the panel via setLog, so it updates
    // in real time as actions occur. The control methods below let tests drive
    // the refresh deterministically offscreen without a live server.

    // Bind to the shared app context (A3): adopts the active schedule id and
    // API base URL and keeps them in sync with context changes. Starting the
    // refresh timer is the default so the hub auto-refreshes in real time.
    void setContext(DemoAppContext *context);
    DemoAppContext *context() const { return m_context; }

    // Explicit control (tests can use these instead of a context).
    void setScheduleId(const QString &id);
    QString scheduleId() const { return m_scheduleId; }
    void setApiBaseUrl(const QString &url);
    QString apiBaseUrl() const { return m_baseUrl; }

    // Refresh timing / control.
    void setRefreshInterval(int ms);
    int refreshInterval() const { return m_timer.interval(); }
    void startRefresh();
    void stopRefresh();
    bool isRefreshing() const { return m_timer.isActive(); }

    // Manual immediate fetch (network).
    void refreshNow();

    // Deterministic refresh path used by tests (no live server): applies a
    // fetched action log to the panel exactly as a successful fetch would, so
    // tests can drive the core refresh logic without network I/O.
    void applyFetchedLog(const QList<AuditLogEntry> &entries);

    // Alias for tests that expect a logRowCount accessor.
    int logRowCount() const { return rowCount(); }

private slots:
    void onPollTick();
    void onActionsReply(QNetworkReply *reply);

private:
    void fetchActions();
    void appendRow(const AuditLogEntry &entry);

    QTableWidget *m_table;
    QNetworkAccessManager m_net;
    QTimer m_timer;
    QString m_baseUrl;
    QString m_scheduleId;
    DemoAppContext *m_context;
};

#endif // PATCHORCHESTRATOR_UI_AUDIT_LOG_PANEL_HPP
