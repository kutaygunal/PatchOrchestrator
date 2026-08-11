// PatchOrchestrator — Sprint 19 (C2) color-coded state tests (Qt Test).
//
// Covers mapping each patch state to a color applied to rows/badges in the
// dashboard:
//   * T1 (c2_mapping) — each state maps to the correct color; unknown/empty
//          states map to a defined default color (no crash).
//   * T2 (c2_consistency) — each endpoint row/badge uses the color for its
//          state; the same state always produces the same color across rows;
//          the color is applied to the correct visual element (state cell).
//   * T3 (c2_regression) — the dashboard still renders endpoint data correctly
//          and the existing P8/C1 behavior (poll timer, progress bars) is
//          preserved.
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

class C2ColorStateTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_stateColorMapping();
    void t2_appliedConsistently();
    void t3_regression();
};

void C2ColorStateTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void C2ColorStateTests::t1_stateColorMapping()
{
    // Each state maps to the correct color.
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("succeeded")),
             QColor(0x2e, 0x7d, 0x32));  // green
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("failed")),
             QColor(0xc6, 0x28, 0x28));  // red
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("paused")),
             QColor(0xf9, 0xa8, 0x25));  // amber
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("running")),
             QColor(0x15, 0x65, 0xc0));  // blue
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("pending")),
             QColor(0x9e, 0x9e, 0x9e));  // grey
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("rolled_back")),
             QColor(0x6a, 0x1b, 0x9a));  // purple

    // Unknown/empty states map to a defined default color (no crash).
    const QColor defaultColor = DashboardWindow::colorForState(QStringLiteral("unknown"));
    QVERIFY(defaultColor.isValid());
    QCOMPARE(defaultColor, DashboardWindow::colorForState(QString()));
    QCOMPARE(defaultColor, DashboardWindow::colorForState(QStringLiteral("bogus_state")));
}

void C2ColorStateTests::t2_appliedConsistently()
{
    DashboardWindow w;
    w.stopPolling();  // isolate the event path from the poll timer

    // Two endpoints with the same state must get the same color; a different
    // state must get a different color.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-3"), QStringLiteral("failed")}}));

    QCOMPARE(w.rowCount(), 3);

    // The color is applied to the correct visual element (the state cell).
    QVERIFY(w.rowStateColor(0).isValid());
    QVERIFY(w.rowStateColor(1).isValid());
    QVERIFY(w.rowStateColor(2).isValid());

    // Same state -> same color across rows.
    QCOMPARE(w.rowStateColor(0), w.rowStateColor(1));
    QCOMPARE(w.rowStateColor(0), DashboardWindow::colorForState(QStringLiteral("succeeded")));

    // Different state -> different color.
    QCOMPARE(w.rowStateColor(2), DashboardWindow::colorForState(QStringLiteral("failed")));
    QVERIFY(w.rowStateColor(2) != w.rowStateColor(0));

    // Out-of-range row returns an invalid color (no crash).
    QVERIFY(!w.rowStateColor(99).isValid());
}

void C2ColorStateTests::t3_regression()
{
    DashboardWindow w;

    // Existing behavior preserved: the poll timer is active by default.
    QVERIFY(w.isPolling());

    // The dashboard still renders endpoint data correctly.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("failed")}}));
    QCOMPARE(w.rowCount(), 2);
    QCOMPARE(w.cellText(0, 0), QStringLiteral("ep-1"));
    QCOMPARE(w.cellText(0, 1), QStringLiteral("succeeded"));
    QCOMPARE(w.cellText(1, 0), QStringLiteral("ep-2"));
    QCOMPARE(w.cellText(1, 1), QStringLiteral("failed"));

    // C1 progress bars are still present and animating to their targets.
    QCOMPARE(w.progressBarCount(), 2);
    QCOMPARE(w.progressBarTarget(0), 50);
    QCOMPARE(w.progressBarTarget(1), 50);

    // C2 colors are still applied alongside the existing rendering.
    QCOMPARE(w.rowStateColor(0), DashboardWindow::colorForState(QStringLiteral("succeeded")));
    QCOMPARE(w.rowStateColor(1), DashboardWindow::colorForState(QStringLiteral("failed")));
}

QTEST_MAIN(C2ColorStateTests)
#include "c2_color_state_tests.moc"
