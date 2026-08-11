// PatchOrchestrator — Phase 8 Qt dashboard (read-only).
//
// A QMainWindow that lists simulated endpoints and their patch status in a
// table, polling/refreshing against the .NET API boundary (Phase 7).

#ifndef PATCHORCHESTRATOR_UI_DASHBOARD_HPP
#define PATCHORCHESTRATOR_UI_DASHBOARD_HPP

#include <QColor>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QVector>

class QCloseEvent;
class QJsonArray;
class QNetworkReply;
class QTableWidget;
class DemoAppContext;
class AnimatedProgressBar;
class FleetSummaryPanel;

class DashboardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DashboardWindow(QWidget *parent = nullptr);

    // Sprint 3 (A3): bind this panel to the shared app context. The dashboard
    // reads the schedule id and API base URL from the context and publishes
    // the observed rollout state back to it.
    void setContext(DemoAppContext *context);
    DemoAppContext *context() const { return m_context; }

    // Polling / refresh control.
    void setPollIntervalMs(int ms);
    void refreshNow();

    // Sprint 16 (B6): live reaction to pause/resume/rollback events.
    //
    // Opens the B5 SSE status stream (GET /api/schedules/{id}/status/stream)
    // and re-renders the table immediately whenever a live state-change event
    // arrives, in addition to (not instead of) the poll timer. This lets the
    // dashboard react to pause/resume/rollback without waiting for the next
    // poll tick.
    void startStatusStream();

    // Process a single status-stream event (a JSON object carrying "status"
    // and/or "endpoints"). Public so tests can simulate an event arriving on
    // the stream without a live server.
    void handleStreamEvent(const QJsonObject &event);

    // Test/control helpers: stop the poll timer and query its state.
    void stopPolling();
    bool isPolling() const { return m_timer.isActive(); }

    // Test accessors for the rendered table.
    int rowCount() const;
    QString cellText(int row, int col) const;

    // Sprint 18 (C1): animated progress bar accessors for tests.
    int progressBarCount() const;
    int progressBarValue(int row) const;
    int progressBarTarget(int row) const;

    // Sprint 19 (C2): centralized state->color mapping (single source of
    // truth). Maps each patch state to a color applied to rows/badges:
    //   succeeded=green, failed=red, paused=amber, running=blue,
    //   pending=grey, rolled_back=purple. Unknown/empty states map to a
    //   defined default color (never crashes).
    //
    // Sprint 20 (C3): the mapping now lives in the reusable StateBadge
    // widget; the dashboard delegates to it so every row/badge stays
    // consistent with the badge.
    static QColor colorForState(const QString &state);

    // Test accessor: the color currently applied to the state cell (badge)
    // of the given row. Returns an invalid QColor if the row is out of range.
    QColor rowStateColor(int row) const;

    // Sprint 20 (C3): test accessor — the StateBadge widget rendered in the
    // state cell of the given row (nullptr if none).
    QWidget *rowStateBadge(int row) const;

    // Sprint 21 (C4): test accessor — the fleet summary panel (never null
    // after construction).
    FleetSummaryPanel *summaryPanel() const { return m_summary; }

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onPollTick();
    void onSimulateReply(QNetworkReply *reply);
    void onStatusReply(QNetworkReply *reply);
    void onCreateReply(QNetworkReply *reply);
    void onStreamReadyRead(QNetworkReply *reply);
    void onStreamFinished(QNetworkReply *reply);

private:
    void buildUi();
    void ensureSchedule();
    void pollSimulate();
    void pollStatus();
    void populateTable(const QJsonArray &endpoints);
    void setStatusMessage(const QString &message);

    QTableWidget *m_table;
    QNetworkAccessManager m_net;
    QTimer m_timer;
    QString m_baseUrl;
    QString m_scheduleId;
    bool m_scheduleReady;
    DemoAppContext *m_context;

    // Sprint 18 (C1): one animated progress bar per endpoint row.
    QVector<AnimatedProgressBar *> m_progressBars;

    // Sprint 16 (B6): live status-stream state.
    QNetworkReply *m_streamReply;
    QByteArray m_streamBuffer;

    // Sprint 21 (C4): fleet summary panel aggregating counts by state.
    FleetSummaryPanel *m_summary;
};

#endif // PATCHORCHESTRATOR_UI_DASHBOARD_HPP
