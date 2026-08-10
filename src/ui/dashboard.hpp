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

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onPollTick();
    void onSimulateReply(QNetworkReply *reply);
    void onStatusReply(QNetworkReply *reply);
    void onCreateReply(QNetworkReply *reply);

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
};

#endif // PATCHORCHESTRATOR_UI_DASHBOARD_HPP
