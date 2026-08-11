// PatchOrchestrator — Sprint 24 (C7) dashboard legend tests (Qt Test).
//
// Covers the dashboard legend that explains the color coding and state
// meanings for demo viewers:
//   * T1 (c7_legend_renders) — the legend is visible in the dashboard and
//          renders without crashing.
//   * T2 (c7_all_states) — the legend includes all six states; each entry
//          shows the correct color (from the C2 mapping) and its meaning.
//   * T3 (c7_regression) — the dashboard still renders endpoint data correctly
//          with the legend present; existing P8/C1/C2/C3/C4/C5/C6 behavior is
//          preserved.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/dashboard.hpp"
#include "ui/dashboard_legend.hpp"
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

class C7DashboardLegendTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_legendRenders();
    void t2_allStates();
    void t3_regression();
};

void C7DashboardLegendTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void C7DashboardLegendTests::t1_legendRenders()
{
    DashboardWindow w;
    w.stopPolling();  // isolate from the poll timer

    // The legend widget exists and is attached to the dashboard.
    DashboardLegend *legend = w.legend();
    QVERIFY(legend != nullptr);

    // The legend has a title and renders without crashing.
    QVERIFY(!legend->title().isEmpty());

    // The legend is visible once the dashboard is shown.
    w.show();
    QVERIFY(legend->isVisible());

    // Rendering the legend (paint) does not crash.
    legend->repaint();
    QVERIFY(legend->entryCount() > 0);
}

void C7DashboardLegendTests::t2_allStates()
{
    DashboardWindow w;
    w.stopPolling();

    DashboardLegend *legend = w.legend();
    QVERIFY(legend != nullptr);

    // The legend includes all six states.
    const QStringList expected = StateBadge::legendStates();
    QCOMPARE(expected.size(), 6);
    QCOMPARE(legend->entryCount(), expected.size());

    for (int i = 0; i < expected.size(); ++i) {
        const QString &state = expected.at(i);

        // Each state entry is present in the legend.
        QVERIFY2(legend->entryState(i) == state,
                 qPrintable(QStringLiteral("legend entry %1 should be %2")
                                .arg(i).arg(state)));

        // Each state entry shows the correct color (from the C2 mapping).
        QCOMPARE(legend->entryColor(i), StateBadge::colorForState(state));
        QCOMPARE(legend->entryColor(i), DashboardWindow::colorForState(state));

        // Each state entry shows its meaning/label.
        QVERIFY2(!legend->entryLabel(i).isEmpty(),
                 qPrintable(QStringLiteral("legend entry %1 has no label").arg(i)));
        QCOMPARE(legend->entryLabel(i), StateBadge::labelForState(state));
    }

    // The six known states are exactly the expected set.
    QVERIFY(expected.contains(QStringLiteral("succeeded")));
    QVERIFY(expected.contains(QStringLiteral("failed")));
    QVERIFY(expected.contains(QStringLiteral("paused")));
    QVERIFY(expected.contains(QStringLiteral("running")));
    QVERIFY(expected.contains(QStringLiteral("pending")));
    QVERIFY(expected.contains(QStringLiteral("rolled_back")));

    // Out-of-range access returns safe defaults (no crash).
    QVERIFY(!legend->entryColor(99).isValid());
    QVERIFY(legend->entryState(99).isEmpty());
    QVERIFY(legend->entryLabel(99).isEmpty());
}

void C7DashboardLegendTests::t3_regression()
{
    DashboardWindow w;
    w.stopPolling();  // isolate the event path from the poll timer

    // The legend is present alongside the existing dashboard content.
    QVERIFY(w.legend() != nullptr);
    QVERIFY(w.summaryPanel() != nullptr);

    // The dashboard still renders endpoint data correctly with the legend.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("failed")},
         {QStringLiteral("ep-3"), QStringLiteral("pending")}}));
    QCOMPARE(w.rowCount(), 3);
    QCOMPARE(w.cellText(0, 0), QStringLiteral("ep-1"));
    QCOMPARE(w.cellText(0, 1), QStringLiteral("succeeded"));
    QCOMPARE(w.cellText(1, 0), QStringLiteral("ep-2"));
    QCOMPARE(w.cellText(1, 1), QStringLiteral("failed"));
    QCOMPARE(w.cellText(2, 0), QStringLiteral("ep-3"));
    QCOMPARE(w.cellText(2, 1), QStringLiteral("pending"));

    // C1 progress bars are still present and animating to their targets.
    QCOMPARE(w.progressBarCount(), 3);
    QCOMPARE(w.progressBarTarget(0), 50);
    QCOMPARE(w.progressBarTarget(1), 50);
    QCOMPARE(w.progressBarTarget(2), 50);

    // C2 colors are still applied alongside the legend.
    QCOMPARE(w.rowStateColor(0), DashboardWindow::colorForState(QStringLiteral("succeeded")));
    QCOMPARE(w.rowStateColor(1), DashboardWindow::colorForState(QStringLiteral("failed")));
    QCOMPARE(w.rowStateColor(2), DashboardWindow::colorForState(QStringLiteral("pending")));

    // C3 badges are still rendered in the state cells.
    QVERIFY(w.rowStateBadge(0) != nullptr);
    QVERIFY(w.rowStateBadge(1) != nullptr);
    QVERIFY(w.rowStateBadge(2) != nullptr);

    // C4 fleet summary still aggregates counts by state.
    QCOMPARE(w.summaryPanel()->countForState(QStringLiteral("succeeded")), 1);
    QCOMPARE(w.summaryPanel()->countForState(QStringLiteral("failed")), 1);
    QCOMPARE(w.summaryPanel()->countForState(QStringLiteral("pending")), 1);
    QCOMPARE(w.summaryPanel()->total(), 3);

    // The legend colors stay in sync with the dashboard/badge mapping.
    QCOMPARE(w.legend()->entryColor(0),
             DashboardWindow::colorForState(QStringLiteral("succeeded")));
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("succeeded")),
             StateBadge::colorForState(QStringLiteral("succeeded")));
}

QTEST_MAIN(C7DashboardLegendTests)
#include "c7_dashboard_legend_tests.moc"
