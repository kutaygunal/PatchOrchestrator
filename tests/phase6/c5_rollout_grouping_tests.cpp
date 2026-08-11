// PatchOrchestrator — Sprint 22 (C5) rollout-stage grouping tests (Qt Test).
//
// Covers grouping endpoints in the dashboard by rollout stage (wave/group)
// with stage headers and per-stage progress:
//   * T1 (c5_grouping) — endpoints are grouped by their stage; each stage
//          contains the correct endpoints; stages appear in the correct order.
//   * T2 (c5_headers) — each stage has a header (its id) displayed above its
//          group of endpoints; the number of headers matches the number of
//          stages.
//   * T3 (c5_progress) — each stage shows its own aggregated progress computed
//          from its endpoints; progress updates when endpoint data changes.
//   * T4 (c5_regression) — the dashboard still renders endpoint data correctly
//          with grouping; the original flat behavior (P8/C1/C2/C3/C4) is
//          preserved when no stages are set.
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
// { "id": ..., "status": ..., "endpoints": [ {id,state,group_id,progress}, ... ] }.
QJsonObject makeEvent(const QString &status, const QJsonArray &endpoints)
{
    return QJsonObject{
        {QStringLiteral("id"), QStringLiteral("sch-1")},
        {QStringLiteral("status"), status},
        {QStringLiteral("endpoints"), endpoints},
    };
}

// Build a single endpoint object carrying a group_id (used for stage grouping).
QJsonObject ep(const QString &id, const QString &state, const QString &group,
               double progress)
{
    return QJsonObject{
        {QStringLiteral("id"), id},
        {QStringLiteral("state"), state},
        {QStringLiteral("group_id"), group},
        {QStringLiteral("progress"), progress},
    };
}

}  // namespace

class C5RolloutGroupingTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_grouping();
    void t2_headers();
    void t3_progress();
    void t4_regression();
};

void C5RolloutGroupingTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void C5RolloutGroupingTests::t1_grouping()
{
    DashboardWindow w;
    w.stopPolling();  // isolate the event path from the poll timer

    // Two stages: stage-1 (grp-a, grp-b), stage-2 (grp-c).
    w.setStages({
        {QStringLiteral("stage-1"), 1, {QStringLiteral("grp-a"), QStringLiteral("grp-b")}},
        {QStringLiteral("stage-2"), 2, {QStringLiteral("grp-c")}},
    });

    QJsonArray endpoints;
    endpoints.append(ep(QStringLiteral("ep-1"), QStringLiteral("succeeded"),
                        QStringLiteral("grp-a"), 100));
    endpoints.append(ep(QStringLiteral("ep-2"), QStringLiteral("running"),
                        QStringLiteral("grp-c"), 50));
    endpoints.append(ep(QStringLiteral("ep-3"), QStringLiteral("pending"),
                        QStringLiteral("grp-b"), 0));
    endpoints.append(ep(QStringLiteral("ep-4"), QStringLiteral("failed"),
                        QStringLiteral("grp-c"), 25));
    w.handleStreamEvent(makeEvent(QStringLiteral("running"), endpoints));

    // Two stages rendered.
    QCOMPARE(w.stageCount(), 2);

    // stage-1 contains ep-1 and ep-3; stage-2 contains ep-2 and ep-4.
    QCOMPARE(w.stageEndpointCount(0), 2);
    QCOMPARE(w.stageEndpointCount(1), 2);

    // Layout: row0 header(stage-1), row1 ep-1, row2 ep-3,
    //         row3 header(stage-2), row4 ep-2, row5 ep-4.
    QCOMPARE(w.endpointStageIndex(1), 0);
    QCOMPARE(w.endpointStageIndex(2), 0);
    QCOMPARE(w.endpointStageIndex(4), 1);
    QCOMPARE(w.endpointStageIndex(5), 1);

    // Header rows are not endpoint rows.
    QCOMPARE(w.endpointStageIndex(0), -1);
    QCOMPARE(w.endpointStageIndex(3), -1);

    // Stages appear in the correct order (stage-1 header before stage-2).
    QVERIFY(w.stageRow(0) < w.stageRow(1));
}

void C5RolloutGroupingTests::t2_headers()
{
    DashboardWindow w;
    w.stopPolling();
    w.setStages({
        {QStringLiteral("stage-1"), 1, {QStringLiteral("grp-a")}},
        {QStringLiteral("stage-2"), 2, {QStringLiteral("grp-b")}},
    });

    QJsonArray endpoints;
    endpoints.append(ep(QStringLiteral("ep-1"), QStringLiteral("succeeded"),
                        QStringLiteral("grp-a"), 100));
    endpoints.append(ep(QStringLiteral("ep-2"), QStringLiteral("running"),
                        QStringLiteral("grp-b"), 50));
    w.handleStreamEvent(makeEvent(QStringLiteral("running"), endpoints));

    // The number of headers matches the number of stages.
    QCOMPARE(w.stageCount(), 2);

    // Each stage has a header containing its id.
    QVERIFY(w.stageHeaderText(0).contains(QStringLiteral("stage-1")));
    QVERIFY(w.stageHeaderText(1).contains(QStringLiteral("stage-2")));

    // Each header is displayed above its group of endpoints.
    QCOMPARE(w.stageRow(0), 0);
    QCOMPARE(w.stageRow(1), 2);  // after stage-1 header + its endpoint
    QVERIFY(w.stageRow(0) < w.stageRow(1));
}

void C5RolloutGroupingTests::t3_progress()
{
    DashboardWindow w;
    w.stopPolling();
    w.setStages({
        {QStringLiteral("stage-1"), 1, {QStringLiteral("grp-a")}},
        {QStringLiteral("stage-2"), 2, {QStringLiteral("grp-b")}},
    });

    // stage-1: ep-1 (100), ep-2 (50) -> avg 75. stage-2: ep-3 (0) -> 0.
    QJsonArray endpoints;
    endpoints.append(ep(QStringLiteral("ep-1"), QStringLiteral("succeeded"),
                        QStringLiteral("grp-a"), 100));
    endpoints.append(ep(QStringLiteral("ep-2"), QStringLiteral("running"),
                        QStringLiteral("grp-a"), 50));
    endpoints.append(ep(QStringLiteral("ep-3"), QStringLiteral("pending"),
                        QStringLiteral("grp-b"), 0));
    w.handleStreamEvent(makeEvent(QStringLiteral("running"), endpoints));

    QCOMPARE(w.stageProgress(0), 75);
    QCOMPARE(w.stageProgress(1), 0);

    // Progress updates when endpoint data changes.
    QJsonArray updated;
    updated.append(ep(QStringLiteral("ep-1"), QStringLiteral("succeeded"),
                      QStringLiteral("grp-a"), 100));
    updated.append(ep(QStringLiteral("ep-2"), QStringLiteral("succeeded"),
                      QStringLiteral("grp-a"), 100));
    updated.append(ep(QStringLiteral("ep-3"), QStringLiteral("running"),
                      QStringLiteral("grp-b"), 50));
    w.handleStreamEvent(makeEvent(QStringLiteral("running"), updated));

    QCOMPARE(w.stageProgress(0), 100);
    QCOMPARE(w.stageProgress(1), 50);
}

void C5RolloutGroupingTests::t4_regression()
{
    // Without stages, the dashboard keeps the original flat behavior.
    DashboardWindow w;
    w.stopPolling();
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        QJsonArray{
            ep(QStringLiteral("ep-1"), QStringLiteral("succeeded"),
               QStringLiteral("grp-a"), 100),
            ep(QStringLiteral("ep-2"), QStringLiteral("failed"),
               QStringLiteral("grp-b"), 0),
        }));
    QCOMPARE(w.rowCount(), 2);
    QCOMPARE(w.cellText(0, 0), QStringLiteral("ep-1"));
    QCOMPARE(w.cellText(0, 1), QStringLiteral("succeeded"));
    QCOMPARE(w.cellText(1, 0), QStringLiteral("ep-2"));
    QCOMPARE(w.cellText(1, 1), QStringLiteral("failed"));
    QCOMPARE(w.progressBarCount(), 2);
    QCOMPARE(w.progressBarTarget(0), 100);
    QCOMPARE(w.rowStateColor(0), DashboardWindow::colorForState(QStringLiteral("succeeded")));
    QVERIFY(w.rowStateBadge(0) != nullptr);

    // With stages, endpoints are grouped and still render correctly.
    DashboardWindow g;
    g.stopPolling();
    g.setStages({
        {QStringLiteral("stage-1"), 1, {QStringLiteral("grp-a")}},
        {QStringLiteral("stage-2"), 2, {QStringLiteral("grp-b")}},
    });
    g.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        QJsonArray{
            ep(QStringLiteral("ep-1"), QStringLiteral("succeeded"),
               QStringLiteral("grp-a"), 100),
            ep(QStringLiteral("ep-2"), QStringLiteral("failed"),
               QStringLiteral("grp-b"), 0),
        }));
    QCOMPARE(g.stageCount(), 2);
    QCOMPARE(g.stageEndpointCount(0), 1);
    QCOMPARE(g.stageEndpointCount(1), 1);

    // Endpoint data (id/state) is still rendered under each stage header.
    QCOMPARE(g.cellText(g.stageRow(0) + 1, 0), QStringLiteral("ep-1"));
    QCOMPARE(g.cellText(g.stageRow(0) + 1, 1), QStringLiteral("succeeded"));
    QCOMPARE(g.cellText(g.stageRow(1) + 1, 0), QStringLiteral("ep-2"));
    QCOMPARE(g.cellText(g.stageRow(1) + 1, 1), QStringLiteral("failed"));

    // The fleet summary is still present and correct with grouping.
    QVERIFY(g.summaryPanel() != nullptr);
    QCOMPARE(g.summaryPanel()->countForState(QStringLiteral("succeeded")), 1);
    QCOMPARE(g.summaryPanel()->countForState(QStringLiteral("failed")), 1);
    QCOMPARE(g.summaryPanel()->total(), 2);
}

QTEST_MAIN(C5RolloutGroupingTests)
#include "c5_rollout_grouping_tests.moc"
