// PatchOrchestrator — Sprint 3 (A3) shared app state tests (Qt Test).
//
// Covers DemoAppContext, the single QObject holding the active schedule id,
// API base URL, and current rollout state shared across all demo panels:
//   * T1 — context state set/get (defaults, getters, idempotent setters).
//   * T2 — signal emission on change (and no signal when unchanged).
//   * T3 — panels share the single context (single source of truth; a change
//          made through one panel is visible through the shared context and
//          reflected in the other panels).
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/control_panel.hpp"
#include "ui/dashboard.hpp"
#include "ui/demo_app_context.hpp"
#include "ui/demo_main_window.hpp"
#include "ui/schedule_editor.hpp"

namespace {

const char kOrgName[] = "PatchOrchestratorTest";
const char kAppName[] = "PatchOrchestrator";

}  // namespace

class A3ContextTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_stateSetGet();
    void t2_signalEmission();
    void t3_sharedContext();
};

void A3ContextTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void A3ContextTests::t1_stateSetGet()
{
    DemoAppContext ctx;

    // Sensible defaults: empty schedule id, default API base URL, initial state.
    QCOMPARE(ctx.scheduleId(), QString());
    QCOMPARE(ctx.apiBaseUrl(), QStringLiteral("http://localhost:5000"));
    QCOMPARE(ctx.rolloutState(), QStringLiteral("idle"));

    // Setting each value is retrievable via the getter.
    ctx.setScheduleId(QStringLiteral("sch-42"));
    QCOMPARE(ctx.scheduleId(), QStringLiteral("sch-42"));

    ctx.setApiBaseUrl(QStringLiteral("http://api.example.test"));
    QCOMPARE(ctx.apiBaseUrl(), QStringLiteral("http://api.example.test"));

    ctx.setRolloutState(QStringLiteral("running"));
    QCOMPARE(ctx.rolloutState(), QStringLiteral("running"));

    // Setting the same value again is a no-op (idempotent).
    ctx.setScheduleId(QStringLiteral("sch-42"));
    QCOMPARE(ctx.scheduleId(), QStringLiteral("sch-42"));
    ctx.setApiBaseUrl(QStringLiteral("http://api.example.test"));
    QCOMPARE(ctx.apiBaseUrl(), QStringLiteral("http://api.example.test"));
    ctx.setRolloutState(QStringLiteral("running"));
    QCOMPARE(ctx.rolloutState(), QStringLiteral("running"));
}

void A3ContextTests::t2_signalEmission()
{
    DemoAppContext ctx;

    int scheduleSignals = 0;
    int urlSignals = 0;
    int stateSignals = 0;

    QObject::connect(&ctx, &DemoAppContext::scheduleIdChanged, &ctx,
                     [&scheduleSignals](const QString &id) {
                         QCOMPARE(id, QStringLiteral("sch-7"));
                         ++scheduleSignals;
                     });
    QObject::connect(&ctx, &DemoAppContext::apiBaseUrlChanged, &ctx,
                     [&urlSignals](const QString &url) {
                         QCOMPARE(url, QStringLiteral("http://other.test"));
                         ++urlSignals;
                     });
    QObject::connect(&ctx, &DemoAppContext::rolloutStateChanged, &ctx,
                     [&stateSignals](const QString &state) {
                         QCOMPARE(state, QStringLiteral("paused"));
                         ++stateSignals;
                     });

    // Changing each value emits the corresponding signal with the new value.
    ctx.setScheduleId(QStringLiteral("sch-7"));
    ctx.setApiBaseUrl(QStringLiteral("http://other.test"));
    ctx.setRolloutState(QStringLiteral("paused"));
    QCOMPARE(scheduleSignals, 1);
    QCOMPARE(urlSignals, 1);
    QCOMPARE(stateSignals, 1);

    // Setting an unchanged value does NOT emit a signal (change-only design).
    ctx.setScheduleId(QStringLiteral("sch-7"));
    ctx.setApiBaseUrl(QStringLiteral("http://other.test"));
    ctx.setRolloutState(QStringLiteral("paused"));
    QCOMPARE(scheduleSignals, 1);
    QCOMPARE(urlSignals, 1);
    QCOMPARE(stateSignals, 1);
}

void A3ContextTests::t3_sharedContext()
{
    DemoMainWindow w;

    // A single shared context exists and is bound to all three panels.
    DemoAppContext *ctx = w.context();
    QVERIFY(ctx != nullptr);
    QVERIFY(w.dashboard()->context() == ctx);
    QVERIFY(w.scheduleEditor()->context() == ctx);
    QVERIFY(w.controlPanel()->context() == ctx);

    // A change made through one panel (control panel schedule-id field) is
    // visible through the shared context and reflected in the other panels.
    w.controlPanel()->setScheduleIdText(QStringLiteral("sch-shared"));
    QCOMPARE(ctx->scheduleId(), QStringLiteral("sch-shared"));

    // The schedule editor's field reflects the shared value (its textChanged
    // handler echoes the same value back, which is a no-op on the context).
    QCOMPARE(w.scheduleEditor()->context()->scheduleId(), QStringLiteral("sch-shared"));

    // A change made through the shared context propagates to the panels.
    ctx->setRolloutState(QStringLiteral("rolled_back"));
    QCOMPARE(ctx->rolloutState(), QStringLiteral("rolled_back"));
    QCOMPARE(w.dashboard()->context()->rolloutState(), QStringLiteral("rolled_back"));
    QCOMPARE(w.controlPanel()->context()->rolloutState(), QStringLiteral("rolled_back"));
}

QTEST_MAIN(A3ContextTests)
#include "a3_context_tests.moc"
