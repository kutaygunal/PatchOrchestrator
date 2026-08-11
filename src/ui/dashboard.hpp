// PatchOrchestrator — Phase 8 Qt dashboard (read-only).
//
// A QMainWindow that lists simulated endpoints and their patch status in a
// table, polling/refreshing against the .NET API boundary (Phase 7).

#ifndef PATCHORCHESTRATOR_UI_DASHBOARD_HPP
#define PATCHORCHESTRATOR_UI_DASHBOARD_HPP

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QTimer>

class QCloseEvent;
class QJsonArray;
class QNetworkReply;
class QTableWidget;
class DemoAppContext;

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

    // Sprint 16 (B6): live status-stream state.
    QNetworkReply *m_streamReply;
    QByteArray m_streamBuffer;
};

#endif // PATCHORCHESTRATOR_UI_DASHBOARD_HPP
