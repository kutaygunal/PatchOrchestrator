// PatchOrchestrator — Phase 3 (P3) dashboard discovery-logic tests (Qt Test).
//
// P3 requires the dashboard to auto-discover its fleet from the API instead of
// hardcoding endpoints: on startup/refresh it calls GET /api/schedules, picks
// the most recently created schedule, and loads that schedule's fleet — while
// preserving the existing PATCHORCH_SCHEDULE_ID env-var override.
//
// Testable contract (engineer must add):
//   * DashboardWindow::static QString resolveScheduleId(
//         const QJsonArray &schedules, const QString &envOverride)
//     Pure discovery logic (no I/O). Returns:
//       - envOverride if it is non-empty (PATCHORCH_SCHEDULE_ID preserved);
//       - otherwise the "id" of the schedule with the newest "created" timestamp;
//       - an empty string when schedules is empty.
//     The dashboard uses this to decide which schedule to load from the API.
//   * DashboardWindow::QString scheduleId() const  (public test accessor)
//     Returns the currently selected schedule id (m_scheduleId).
//
// Tests:
//   * T1 — env override wins over the newest schedule.
//   * T2 — picks the newest schedule by created timestamp when no override.
//   * T3 — discovery is order-independent (unsorted list still returns newest).
//   * T4 — an empty schedule list resolves to an empty schedule id.
//   * T5 — the dashboard honors the PATCHORCH_SCHEDULE_ID env var on startup.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QJsonArray>
#include <QJsonObject>

#include "ui/dashboard.hpp"

class P3DashboardDiscoveryTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void t1_envOverrideWins();
    void t2_picksNewest();
    void t3_orderIndependent();
    void t4_emptyList();
    void t5_envOverrideOnStartup();
};

void P3DashboardDiscoveryTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String("PatchOrchestratorTest"));
    QCoreApplication::setApplicationName(QLatin1String("PatchOrchestrator"));
}

void P3DashboardDiscoveryTests::cleanupTestCase()
{
    qunsetenv("PATCHORCH_SCHEDULE_ID");
}

// Build a schedule summary list (as returned by GET /api/schedules) with the
// given ids in the given creation order (oldest first). Each element carries
// id, status, and created (the P1 list contract).
static QJsonArray makeSchedules(const QList<QString> &ids, bool ascending = true)
{
    QJsonArray arr;
    for (int i = 0; i < ids.size(); ++i) {
        const int day = ascending ? (i + 1) : (ids.size() - i);
        QJsonObject o;
        o.insert(QStringLiteral("id"), ids[i]);
        o.insert(QStringLiteral("status"), QStringLiteral("running"));
        // Distinct, correctly-ordered ISO timestamps.
        o.insert(QStringLiteral("created"),
                 QStringLiteral("2024-01-%1T00:00:00Z").arg(day, 2, 10, QLatin1Char('0')));
        arr.append(o);
    }
    return arr;
}

void P3DashboardDiscoveryTests::t1_envOverrideWins()
{
    // Newest schedule is "sch-new", but the env override must win.
    const QJsonArray schedules =
        makeSchedules({QStringLiteral("sch-old"), QStringLiteral("sch-new")});
    QCOMPARE(DashboardWindow::resolveScheduleId(
                 schedules, QStringLiteral("sch-env")),
             QStringLiteral("sch-env"));
}

void P3DashboardDiscoveryTests::t2_picksNewest()
{
    const QJsonArray schedules =
        makeSchedules({QStringLiteral("sch-a"), QStringLiteral("sch-b"),
                       QStringLiteral("sch-c")});
    QCOMPARE(DashboardWindow::resolveScheduleId(schedules, QString()),
             QStringLiteral("sch-c"));
}

void P3DashboardDiscoveryTests::t3_orderIndependent()
{
    // The list must be ordered newest-first by the API, but discovery must not
    // rely on that order: pass the newest element last and second-newest first.
    QJsonArray arr;
    QJsonObject newest;
    newest.insert(QStringLiteral("id"), QStringLiteral("sch-newest"));
    newest.insert(QStringLiteral("status"), QStringLiteral("running"));
    newest.insert(QStringLiteral("created"), QStringLiteral("2024-06-15T00:00:00Z"));
    QJsonObject mid;
    mid.insert(QStringLiteral("id"), QStringLiteral("sch-mid"));
    mid.insert(QStringLiteral("status"), QStringLiteral("running"));
    mid.insert(QStringLiteral("created"), QStringLiteral("2024-03-10T00:00:00Z"));
    QJsonObject oldest;
    oldest.insert(QStringLiteral("id"), QStringLiteral("sch-oldest"));
    oldest.insert(QStringLiteral("status"), QStringLiteral("running"));
    oldest.insert(QStringLiteral("created"), QStringLiteral("2024-01-01T00:00:00Z"));
    arr.append(mid);     // second-newest first
    arr.append(oldest);  // oldest in the middle
    arr.append(newest);  // newest last

    QCOMPARE(DashboardWindow::resolveScheduleId(arr, QString()),
             QStringLiteral("sch-newest"));
}

void P3DashboardDiscoveryTests::t4_emptyList()
{
    QCOMPARE(DashboardWindow::resolveScheduleId(QJsonArray(), QString()),
             QString());
    QCOMPARE(DashboardWindow::resolveScheduleId(QJsonArray(), QStringLiteral("sch-x")),
             QStringLiteral("sch-x"));
}

void P3DashboardDiscoveryTests::t5_envOverrideOnStartup()
{
    // When PATCHORCH_SCHEDULE_ID is set, the dashboard adopts it as the active
    // schedule id instead of auto-selecting from the API.
    qputenv("PATCHORCH_SCHEDULE_ID", "sch-startup");
    DashboardWindow w;
    w.stopPolling();
    QCOMPARE(w.scheduleId(), QStringLiteral("sch-startup"));
}

QTEST_MAIN(P3DashboardDiscoveryTests)
#include "p3_dashboard_discovery_tests.moc"
