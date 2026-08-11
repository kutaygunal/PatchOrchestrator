// PatchOrchestrator — Sprint 21 (C4) fleet summary panel tests (Qt Test).
//
// Covers the fleet summary widget that aggregates endpoint counts by state
// (succeeded/failed/paused/running/pending/rolled_back) and shows the total:
//   * T1 (c4_aggregation) — the count for each state matches the number of
//          endpoints in that state; the total equals the sum of all per-state
//          counts and the total number of endpoints.
//   * T2 (c4_total) — the total equals the number of endpoints and the sum of
//          the six per-state counts; an empty fleet shows zero counts and a
//          total of zero.
//   * T3 (c4_updates) — when an endpoint's state changes, the summary counts
//          update; adding/removing endpoints updates the counts and total.
//   * T4 (c4_regression) — the dashboard still renders endpoint data correctly
//          with the summary panel; existing P8/C1/C2/C3 behavior is preserved.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/dashboard.hpp"
#include "ui/fleet_summary_panel.hpp"
#include "ui/state_badge.hpp"

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

class C4FleetSummaryTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_aggregation();
    void t2_total();
    void t3_updates();
    void t4_regression();
};

void C4FleetSummaryTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void C4FleetSummaryTests::t1_aggregation()
{
    DashboardWindow w;
    w.stopPolling();  // isolate the event path from the poll timer

    // A fleet with a known distribution across all six states.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-3"), QStringLiteral("failed")},
         {QStringLiteral("ep-4"), QStringLiteral("paused")},
         {QStringLiteral("ep-5"), QStringLiteral("running")},
         {QStringLiteral("ep-6"), QStringLiteral("pending")},
         {QStringLiteral("ep-7"), QStringLiteral("rolled_back")}}));

    FleetSummaryPanel *summary = w.summaryPanel();
    QVERIFY(summary != nullptr);

    // Count per state matches the number of endpoints in that state.
    QCOMPARE(summary->countForState(QStringLiteral("succeeded")), 2);
    QCOMPARE(summary->countForState(QStringLiteral("failed")), 1);
    QCOMPARE(summary->countForState(QStringLiteral("paused")), 1);
    QCOMPARE(summary->countForState(QStringLiteral("running")), 1);
    QCOMPARE(summary->countForState(QStringLiteral("pending")), 1);
    QCOMPARE(summary->countForState(QStringLiteral("rolled_back")), 1);

    // Total equals the sum of all per-state counts and the endpoint count.
    const int sum = summary->countForState(QStringLiteral("succeeded")) +
                    summary->countForState(QStringLiteral("failed")) +
                    summary->countForState(QStringLiteral("paused")) +
                    summary->countForState(QStringLiteral("running")) +
                    summary->countForState(QStringLiteral("pending")) +
                    summary->countForState(QStringLiteral("rolled_back"));
    QCOMPARE(summary->total(), sum);
    QCOMPARE(summary->total(), 7);
    QCOMPARE(w.rowCount(), 7);
}

void C4FleetSummaryTests::t2_total()
{
    DashboardWindow w;
    w.stopPolling();

    FleetSummaryPanel *summary = w.summaryPanel();
    QVERIFY(summary != nullptr);

    // Empty fleet: zero counts and a total of zero.
    w.handleStreamEvent(makeEvent(QStringLiteral("idle"), {}));
    QCOMPARE(summary->total(), 0);
    QCOMPARE(summary->countForState(QStringLiteral("succeeded")), 0);
    QCOMPARE(summary->countForState(QStringLiteral("failed")), 0);
    QCOMPARE(summary->countForState(QStringLiteral("paused")), 0);
    QCOMPARE(summary->countForState(QStringLiteral("running")), 0);
    QCOMPARE(summary->countForState(QStringLiteral("pending")), 0);
    QCOMPARE(summary->countForState(QStringLiteral("rolled_back")), 0);

    // A fleet where every endpoint shares one state.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-3"), QStringLiteral("succeeded")}}));
    QCOMPARE(summary->countForState(QStringLiteral("succeeded")), 3);
    QCOMPARE(summary->total(), 3);
    QCOMPARE(summary->total(), w.rowCount());

    // Total equals the sum of the six per-state counts.
    const int sum = summary->countForState(QStringLiteral("succeeded")) +
                    summary->countForState(QStringLiteral("failed")) +
                    summary->countForState(QStringLiteral("paused")) +
                    summary->countForState(QStringLiteral("running")) +
                    summary->countForState(QStringLiteral("pending")) +
                    summary->countForState(QStringLiteral("rolled_back"));
    QCOMPARE(summary->total(), sum);
}

void C4FleetSummaryTests::t3_updates()
{
    DashboardWindow w;
    w.stopPolling();

    FleetSummaryPanel *summary = w.summaryPanel();
    QVERIFY(summary != nullptr);

    // Initial fleet.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("failed")}}));
    QCOMPARE(summary->countForState(QStringLiteral("succeeded")), 1);
    QCOMPARE(summary->countForState(QStringLiteral("failed")), 1);
    QCOMPARE(summary->total(), 2);

    // An endpoint's state changes: ep-2 failed -> succeeded.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("succeeded")}}));
    QCOMPARE(summary->countForState(QStringLiteral("succeeded")), 2);
    QCOMPARE(summary->countForState(QStringLiteral("failed")), 0);
    QCOMPARE(summary->total(), 2);

    // Adding an endpoint updates the counts and total.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-3"), QStringLiteral("pending")}}));
    QCOMPARE(summary->countForState(QStringLiteral("succeeded")), 2);
    QCOMPARE(summary->countForState(QStringLiteral("pending")), 1);
    QCOMPARE(summary->total(), 3);

    // Removing an endpoint updates the counts and total.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-3"), QStringLiteral("pending")}}));
    QCOMPARE(summary->countForState(QStringLiteral("succeeded")), 1);
    QCOMPARE(summary->countForState(QStringLiteral("pending")), 1);
    QCOMPARE(summary->total(), 2);

    // The summary reflects the latest data.
    QCOMPARE(summary->total(), w.rowCount());
}

void C4FleetSummaryTests::t4_regression()
{
    DashboardWindow w;
    w.stopPolling();

    // The dashboard still renders endpoint data correctly with the summary.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("failed")}}));
    QCOMPARE(w.rowCount(), 2);
    QCOMPARE(w.cellText(0, 0), QStringLiteral("ep-1"));
    QCOMPARE(w.cellText(0, 1), QStringLiteral("succeeded"));
    QCOMPARE(w.cellText(1, 0), QStringLiteral("ep-2"));
    QCOMPARE(w.cellText(1, 1), QStringLiteral("failed"));

    // The summary panel is present and reflects the same data.
    FleetSummaryPanel *summary = w.summaryPanel();
    QVERIFY(summary != nullptr);
    QCOMPARE(summary->countForState(QStringLiteral("succeeded")), 1);
    QCOMPARE(summary->countForState(QStringLiteral("failed")), 1);
    QCOMPARE(summary->total(), 2);

    // C1 progress bars are still present and animating to their targets.
    QCOMPARE(w.progressBarCount(), 2);
    QCOMPARE(w.progressBarTarget(0), 50);
    QCOMPARE(w.progressBarTarget(1), 50);

    // C2 colors are still applied alongside the summary rendering.
    QCOMPARE(w.rowStateColor(0), DashboardWindow::colorForState(QStringLiteral("succeeded")));
    QCOMPARE(w.rowStateColor(1), DashboardWindow::colorForState(QStringLiteral("failed")));

    // C3 badges are still rendered in the state cells.
    QVERIFY(w.rowStateBadge(0) != nullptr);
    QVERIFY(w.rowStateBadge(1) != nullptr);

    // The summary uses the same state set as the C2/C3 mapping.
    QCOMPARE(FleetSummaryPanel::states(), StateBadge::legendStates());
}

QTEST_MAIN(C4FleetSummaryTests)
#include "c4_fleet_summary_tests.moc"
