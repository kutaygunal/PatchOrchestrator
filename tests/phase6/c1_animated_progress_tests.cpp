// PatchOrchestrator — Sprint 18 (C1) animated progress bar tests (Qt Test).
//
// Covers replacing static progress text with an animated QProgressBar per
// endpoint that smoothly animates to the target value:
//   * T1 — the bar animates from its start toward the target, passes through
//          intermediate values (smooth), and equals the target on completion.
//   * T2 — each endpoint in the dashboard has its own progress bar and each
//          bar animates to its own endpoint's target (independent of others).
//   * T3 — when an endpoint's target changes, the bar re-animates to the new
//          target instead of jumping instantly.
//   * T4 — regression: the dashboard still renders endpoint data correctly and
//          the existing refresh path still works.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/animated_progress_bar.hpp"
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

class C1AnimatedProgressTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_animationReachesTarget();
    void t2_perEndpointBars();
    void t3_targetChangeReanimates();
    void t4_regression();
};

void C1AnimatedProgressTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void C1AnimatedProgressTests::t1_animationReachesTarget()
{
    AnimatedProgressBar bar;
    bar.setDurationMs(500);
    QCOMPARE(bar.value(), 0);

    bar.setTarget(80);
    QVERIFY(bar.isAnimating());

    // Partway through, the value is strictly between start and target (smooth,
    // not an instant jump).
    QTest::qWait(100);
    const int mid = bar.value();
    QVERIFY(mid > 0 && mid < 80);

    // After the animation completes, the value equals the target.
    QTest::qWait(600);
    QCOMPARE(bar.value(), 80);
    QVERIFY(!bar.isAnimating());
}

void C1AnimatedProgressTests::t2_perEndpointBars()
{
    DashboardWindow w;
    w.stopPolling();  // isolate the event path from the poll timer

    // Three endpoints with distinct targets.
    QJsonArray endpoints;
    endpoints.append(QJsonObject{
        {QStringLiteral("id"), QStringLiteral("ep-1")},
        {QStringLiteral("state"), QStringLiteral("running")},
        {QStringLiteral("progress"), 30.0}});
    endpoints.append(QJsonObject{
        {QStringLiteral("id"), QStringLiteral("ep-2")},
        {QStringLiteral("state"), QStringLiteral("running")},
        {QStringLiteral("progress"), 60.0}});
    endpoints.append(QJsonObject{
        {QStringLiteral("id"), QStringLiteral("ep-3")},
        {QStringLiteral("state"), QStringLiteral("running")},
        {QStringLiteral("progress"), 90.0}});
    w.handleStreamEvent(QJsonObject{
        {QStringLiteral("status"), QStringLiteral("running")},
        {QStringLiteral("endpoints"), endpoints}});

    // One progress bar per endpoint.
    QCOMPARE(w.progressBarCount(), 3);
    QCOMPARE(w.rowCount(), 3);

    // Each bar animates to its own endpoint's target (independent of others).
    QCOMPARE(w.progressBarTarget(0), 30);
    QCOMPARE(w.progressBarTarget(1), 60);
    QCOMPARE(w.progressBarTarget(2), 90);

    // Let them animate; each reaches its own target.
    QTest::qWait(700);
    QCOMPARE(w.progressBarValue(0), 30);
    QCOMPARE(w.progressBarValue(1), 60);
    QCOMPARE(w.progressBarValue(2), 90);
}

void C1AnimatedProgressTests::t3_targetChangeReanimates()
{
    AnimatedProgressBar bar;
    bar.setDurationMs(500);
    bar.setTarget(40);
    QTest::qWait(100);  // partially animated
    const int before = bar.value();
    QVERIFY(before > 0 && before < 40);

    // Change the target mid-animation; the bar re-animates to the new target
    // and does not jump instantly.
    bar.setTarget(90);
    QVERIFY(bar.isAnimating());
    QVERIFY(bar.value() < 90);  // not an instant jump

    QTest::qWait(700);
    QCOMPARE(bar.value(), 90);
    QVERIFY(!bar.isAnimating());
}

void C1AnimatedProgressTests::t4_regression()
{
    DashboardWindow w;

    // Existing behavior preserved: the poll timer is active by default.
    QVERIFY(w.isPolling());

    // The dashboard still renders endpoint data correctly.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("failed")}},
        50.0));
    QCOMPARE(w.rowCount(), 2);
    QCOMPARE(w.cellText(0, 0), QStringLiteral("ep-1"));
    QCOMPARE(w.cellText(0, 1), QStringLiteral("succeeded"));
    QCOMPARE(w.cellText(1, 0), QStringLiteral("ep-2"));
    QCOMPARE(w.cellText(1, 1), QStringLiteral("failed"));

    // Progress bars are present and animating to their targets.
    QCOMPARE(w.progressBarCount(), 2);
    QCOMPARE(w.progressBarTarget(0), 50);
    QCOMPARE(w.progressBarTarget(1), 50);
}

QTEST_MAIN(C1AnimatedProgressTests)
#include "c1_animated_progress_tests.moc"
