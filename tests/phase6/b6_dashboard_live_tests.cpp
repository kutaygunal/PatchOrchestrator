// PatchOrchestrator — Sprint 16 (B6) dashboard live-reaction tests (Qt Test).
//
// Covers the dashboard re-rendering immediately on pause/resume/rollback events
// delivered by the B5 status stream, rather than only on its poll timer:
//   * T1 — an event triggers an immediate re-render reflecting the new state.
//   * T2 — with the poll timer stopped, an incoming event still re-renders
//          (no reliance on the poll timer).
//   * T3 — a sequence of pause/resume/rollback events produces a corresponding
//          sequence of re-renders; the final state matches the last event.
//   * T4 — regression: the poll timer is still active by default and the
//          existing refresh path still works alongside the stream.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/dashboard.hpp"

namespace {

const char kOrgName[] = "PatchOrchestratorTest";
const char kAppName[] = "PatchOrchestrator";

// Build a status-stream event payload matching the B5 SSE format:
// { "id": ..., "status": ..., "endpoints": [ {id,state,progress}, ... ] }.
QJsonObject makeEvent(const QString &status,
                      const QList<QPair<QString, QString>> &endpoints)
{
    QJsonArray arr;
    for (const auto &ep : endpoints) {
        arr.append(QJsonObject{
            {QStringLiteral("id"), ep.first},
            {QStringLiteral("state"), ep.second},
            {QStringLiteral("progress"), 50.0},
        });
    }
    return QJsonObject{
        {QStringLiteral("id"), QStringLiteral("sch-1")},
        {QStringLiteral("status"), status},
        {QStringLiteral("endpoints"), arr},
    };
}

}  // namespace

class B6DashboardLiveTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_eventTriggersRerender();
    void t2_noRelianceOnPollTimer();
    void t3_multipleEvents();
    void t4_regression();
};

void B6DashboardLiveTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void B6DashboardLiveTests::t1_eventTriggersRerender()
{
    DashboardWindow w;
    w.stopPolling();  // isolate the event path from the poll timer

    // The table starts empty (no server, no poll yet).
    QCOMPARE(w.rowCount(), 0);

    // A pause event arrives on the stream.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("paused"),
        {{QStringLiteral("ep-1"), QStringLiteral("paused")},
         {QStringLiteral("ep-2"), QStringLiteral("paused")}}));

    // The dashboard re-rendered immediately, reflecting the new state.
    QCOMPARE(w.rowCount(), 2);
    QCOMPARE(w.cellText(0, 0), QStringLiteral("ep-1"));
    QCOMPARE(w.cellText(0, 1), QStringLiteral("paused"));
    QCOMPARE(w.cellText(1, 0), QStringLiteral("ep-2"));
    QCOMPARE(w.cellText(1, 1), QStringLiteral("paused"));
}

void B6DashboardLiveTests::t2_noRelianceOnPollTimer()
{
    DashboardWindow w;
    w.stopPolling();
    QVERIFY(!w.isPolling());  // poll timer is disabled

    // Even with the poll timer stopped, an incoming stream event re-renders.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")},
         {QStringLiteral("ep-2"), QStringLiteral("running")},
         {QStringLiteral("ep-3"), QStringLiteral("running")}}));

    QCOMPARE(w.rowCount(), 3);
    QCOMPARE(w.cellText(0, 1), QStringLiteral("running"));
    QCOMPARE(w.cellText(2, 1), QStringLiteral("running"));
}

void B6DashboardLiveTests::t3_multipleEvents()
{
    DashboardWindow w;
    w.stopPolling();

    // A sequence of pause -> resume -> rollback events.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("paused"),
        {{QStringLiteral("ep-1"), QStringLiteral("paused")}}));
    QCOMPARE(w.rowCount(), 1);
    QCOMPARE(w.cellText(0, 1), QStringLiteral("paused"));

    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")},
         {QStringLiteral("ep-2"), QStringLiteral("running")}}));
    QCOMPARE(w.rowCount(), 2);
    QCOMPARE(w.cellText(0, 1), QStringLiteral("running"));

    w.handleStreamEvent(makeEvent(
        QStringLiteral("rolled_back"),
        {{QStringLiteral("ep-1"), QStringLiteral("rolled_back")},
         {QStringLiteral("ep-2"), QStringLiteral("rolled_back")},
         {QStringLiteral("ep-3"), QStringLiteral("rolled_back")}}));

    // Final state matches the last event.
    QCOMPARE(w.rowCount(), 3);
    QCOMPARE(w.cellText(0, 1), QStringLiteral("rolled_back"));
    QCOMPARE(w.cellText(1, 1), QStringLiteral("rolled_back"));
    QCOMPARE(w.cellText(2, 1), QStringLiteral("rolled_back"));
}

void B6DashboardLiveTests::t4_regression()
{
    DashboardWindow w;

    // Existing behavior preserved: the poll timer is active by default.
    QVERIFY(w.isPolling());

    // The existing refresh path still works (no crash, table still usable).
    w.refreshNow();

    // The stream event path still works alongside the poll timer.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("paused"),
        {{QStringLiteral("ep-1"), QStringLiteral("paused")}}));
    QCOMPARE(w.rowCount(), 1);
    QCOMPARE(w.cellText(0, 1), QStringLiteral("paused"));
}

QTEST_MAIN(B6DashboardLiveTests)
#include "b6_dashboard_live_tests.moc"
