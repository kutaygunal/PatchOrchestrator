// PatchOrchestrator — Sprint 23 (C6) smooth refresh tests (Qt Test).
//
// Covers the dashboard using a QTimer + interpolation (the C1 animated
// progress bar) so progress bars animate smoothly without flicker, driven by
// the B5 status stream:
//   * T1 (c6_smooth) — a B5 stream event drives the progress bar to a new
//          value smoothly: it passes through intermediate values (no instant
//          jump) and equals the target on completion.
//   * T2 (c6_no_flicker) — during an update the bar never resets or jumps
//          backward; values interpolate monotonically toward the target.
//   * T3 (c6_stream_driven) — a state change on the B5 stream triggers a
//          smooth update without waiting for the poll timer; a sequence of
//          stream events produces a sequence of smooth updates.
//   * T4 (c6_regression) — the dashboard still works with its poll timer and
//          the existing refresh path is preserved.
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
                      const QList<QPair<QString, QString>> &endpoints,
                      double progress)
{
    QJsonArray arr;
    for (const auto &ep : endpoints) {
        arr.append(QJsonObject{
            {QStringLiteral("id"), ep.first},
            {QStringLiteral("state"), ep.second},
            {QStringLiteral("progress"), progress},
        });
    }
    return QJsonObject{
        {QStringLiteral("id"), QStringLiteral("sch-1")},
        {QStringLiteral("status"), status},
        {QStringLiteral("endpoints"), arr},
    };
}

}  // namespace

class C6SmoothRefreshTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_smooth();
    void t2_noFlicker();
    void t3_streamDriven();
    void t4_regression();
};

void C6SmoothRefreshTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

// T1 — Smooth update path: a B5 stream event drives the bar to a new value
// smoothly (intermediate values, no jump), and the update is driven by the
// stream (poll timer stopped).
void C6SmoothRefreshTests::t1_smooth()
{
    DashboardWindow w;
    w.stopPolling();  // isolate the stream path from the poll timer
    QVERIFY(!w.isPolling());

    // Baseline: one endpoint at 0%.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")}},
        0.0));
    QTest::qWait(700);  // let the initial animation settle
    QCOMPARE(w.progressBarValue(0), 0);

    // A stream event raises the target to 80%.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")}},
        80.0));

    // The bar animates toward the target; partway through it is strictly
    // between start and target (smooth, not an instant jump).
    QTest::qWait(100);
    const int mid = w.progressBarValue(0);
    QVERIFY(mid > 0 && mid < 80);

    // After the animation completes, the value equals the target.
    QTest::qWait(600);
    QCOMPARE(w.progressBarValue(0), 80);
}

// T2 — No flicker: during an update the bar never resets or jumps backward;
// values interpolate monotonically toward the target.
void C6SmoothRefreshTests::t2_noFlicker()
{
    DashboardWindow w;
    w.stopPolling();

    // Baseline at 0%.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")}},
        0.0));
    QTest::qWait(700);
    QCOMPARE(w.progressBarValue(0), 0);

    // Raise the target to 100% and sample the value over time.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")}},
        100.0));

    int previous = w.progressBarValue(0);
    for (int i = 0; i < 40; ++i) {
        QTest::qWait(20);
        const int current = w.progressBarValue(0);
        // Monotonic toward the target: never resets or jumps backward.
        QVERIFY2(current >= previous,
                 qPrintable(QStringLiteral("bar jumped backward: %1 -> %2")
                                .arg(previous).arg(current)));
        previous = current;
    }

    // Reached the target without flicker.
    QCOMPARE(w.progressBarValue(0), 100);
}

// T3 — Driven by B5 stream: a state change on the stream triggers a smooth
// update without waiting for the poll timer; multiple stream events produce a
// sequence of smooth updates.
void C6SmoothRefreshTests::t3_streamDriven()
{
    DashboardWindow w;
    w.stopPolling();
    QVERIFY(!w.isPolling());  // no poll timer to drive the update

    // First stream event: 0%.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")}},
        0.0));
    QTest::qWait(700);
    QCOMPARE(w.progressBarValue(0), 0);

    // Second stream event: 50%. The update happens immediately (no poll wait).
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")}},
        50.0));
    QTest::qWait(100);
    const int mid1 = w.progressBarValue(0);
    QVERIFY(mid1 > 0 && mid1 < 50);  // already animating toward 50

    // Third stream event: 90%. A sequence of smooth updates.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("running")}},
        90.0));
    QTest::qWait(100);
    const int mid2 = w.progressBarValue(0);
    QVERIFY(mid2 > 0 && mid2 < 90);

    // Final value matches the last stream event.
    QTest::qWait(700);
    QCOMPARE(w.progressBarValue(0), 90);
}

// T4 — Regression: the dashboard still works with its poll timer and the
// existing refresh path is preserved.
void C6SmoothRefreshTests::t4_regression()
{
    DashboardWindow w;

    // Existing behavior preserved: the poll timer is active by default.
    QVERIFY(w.isPolling());

    // The existing refresh path still works (no crash, table usable).
    w.refreshNow();

    // The stream event path still works alongside the poll timer.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("paused"),
        {{QStringLiteral("ep-1"), QStringLiteral("paused")}},
        40.0));
    QCOMPARE(w.rowCount(), 1);
    QCOMPARE(w.cellText(0, 0), QStringLiteral("ep-1"));
    QCOMPARE(w.cellText(0, 1), QStringLiteral("paused"));

    // The progress bar is present and animating to its target.
    QCOMPARE(w.progressBarCount(), 1);
    QCOMPARE(w.progressBarTarget(0), 40);
}

QTEST_MAIN(C6SmoothRefreshTests)
#include "c6_smooth_refresh_tests.moc"
