// PatchOrchestrator — Phase 8 Qt dashboard (read-only).
//
// A QMainWindow that lists simulated endpoints and their patch status in a
// table, polling/refreshing against the .NET API boundary (Phase 7).

#ifndef PATCHORCHESTRATOR_UI_DASHBOARD_HPP
#define PATCHORCHESTRATOR_UI_DASHBOARD_HPP

#include <QColor>
#include <QJsonArray>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QStringList>
#include <QTimer>
#include <QVector>

class QCloseEvent;
class QJsonArray;
class QNetworkReply;
class QTableWidget;
class DemoAppContext;
class AnimatedProgressBar;
class FleetSummaryPanel;
class DashboardLegend;

// Sprint 22 (C5): a rollout stage (wave/group) as defined by the schedule
// editor (P9). Each stage has an id, an order, and the set of group ids that
// belong to it. Endpoints are grouped by stage in the dashboard.
struct RolloutStage
{
    QString id;
    int order = 0;
    QStringList groupIds;
};

class DashboardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DashboardWindow(QWidget *parent = nullptr);

    // P3: auto-discovery. Decides which schedule the dashboard should load.
    // Returns envOverride when it is non-empty (preserves the
    // PATCHORCH_SCHEDULE_ID override); otherwise the "id" of the schedule with
    // the newest "created" timestamp; empty when schedules is empty. Pure
    // logic (no I/O), so it is unit-testable in isolation.
    static QString resolveScheduleId(const QJsonArray &schedules,
                                     const QString &envOverride);

    // P3: public test accessor for the currently selected schedule id.
    QString scheduleId() const { return m_scheduleId; }

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

    // Sprint 24 (C7): test accessor — the dashboard legend explaining the
    // color coding and state meanings (never null after construction).
    DashboardLegend *legend() const { return m_legend; }

    // Sprint 22 (C5): rollout-stage grouping. Set the stages (from the
    // schedule editor / P9) to group endpoints by stage with stage headers and
    // per-stage progress. Passing an empty list disables grouping (flat table).
    void setStages(const QVector<RolloutStage> &stages);
    QVector<RolloutStage> stages() const { return m_stages; }

    // C5 test accessors (grouped mode).
    int stageCount() const;
    QString stageHeaderText(int stageIndex) const;
    int stageProgress(int stageIndex) const;
    int stageEndpointCount(int stageIndex) const;
    int stageRow(int stageIndex) const;
    int endpointStageIndex(int row) const;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onPollTick();
    void onSimulateReply(QNetworkReply *reply);
    void onStatusReply(QNetworkReply *reply);
    void onSchedulesReply(QNetworkReply *reply);
    void onFleetReply(QNetworkReply *reply);
    void onCreateReply(QNetworkReply *reply);
    void onStreamReadyRead(QNetworkReply *reply);
    void onStreamFinished(QNetworkReply *reply);

private:
    void buildUi();
    void discoverSchedules();
    // Returns the "created" timestamp for the given schedule id, or empty if
    // not present. Used to detect a same-id overwrite and force a reload.
    QString scheduleCreated(const QJsonArray &schedules, const QString &id) const;
    void startWithSchedule();
    void fetchFleet();
    void ensureSchedule();
    void beginPolling();
    void pollSimulate();
    void pollStatus();
    void populateTable(const QJsonArray &endpoints);
    void populateFlat(const QJsonArray &endpoints);
    void populateGrouped(const QJsonArray &endpoints);
    void setStatusMessage(const QString &message);

    QTableWidget *m_table;
    QNetworkAccessManager m_net;
    QTimer m_timer;
    QString m_baseUrl;
    QString m_scheduleId;
    bool m_scheduleReady;
    bool m_scheduleExists; // P3+fix: schedule came from the API (skip re-POST)
    QString m_lastCreated; // last-seen "created" for m_scheduleId (reload on overwrite)
    int m_currentSeed = 42; // schedule's persisted seed (used by pollSimulate)
    QJsonArray m_fleet; // P3: discovered fleet loaded from the API
    DemoAppContext *m_context;

    // Sprint 18 (C1): one animated progress bar per endpoint row.
    QVector<AnimatedProgressBar *> m_progressBars;

    // Sprint 16 (B6): live status-stream state.
    QNetworkReply *m_streamReply;
    QByteArray m_streamBuffer;

    // Sprint 21 (C4): fleet summary panel aggregating counts by state.
    FleetSummaryPanel *m_summary;

    // Sprint 24 (C7): dashboard legend explaining the color coding and state
    // meanings for demo viewers.
    DashboardLegend *m_legend;

    // Sprint 22 (C5): rollout-stage grouping state.
    QVector<RolloutStage> m_stages;
    QVector<int> m_stageRow;           // table row of each stage header
    QVector<int> m_stageEndpointCount; // endpoints per stage
    QVector<int> m_stageProgress;      // aggregated progress per stage (0-100)
    QVector<int> m_endpointStage;      // stage index per table row (-1 = header)
};

#endif // PATCHORCHESTRATOR_UI_DASHBOARD_HPP
