// PatchOrchestrator — Phase 8 Qt dashboard (read-only).
//
// A QMainWindow that lists simulated endpoints and their patch status in a
// table, polling/refreshing against the .NET API boundary (Phase 7).

#ifndef PATCHORCHESTRATOR_UI_DASHBOARD_HPP
#define PATCHORCHESTRATOR_UI_DASHBOARD_HPP

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QTimer>

class QJsonArray;
class QNetworkReply;
class QTableWidget;

class DashboardWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DashboardWindow(QWidget *parent = nullptr);

    // Polling / refresh control.
    void setPollIntervalMs(int ms);
    void refreshNow();

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
};

#endif // PATCHORCHESTRATOR_UI_DASHBOARD_HPP
