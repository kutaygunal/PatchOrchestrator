// PatchOrchestrator — Sprint 2 (A2) tab/dock manager tests (Qt Test).
//
// Covers the central QTabWidget layout manager that embeds the three UI
// widgets and persists layout state across restarts:
//   * T1 — layout save/restore round-trip (non-empty state, restored order,
//          idempotent save -> restore -> save).
//   * T2 — widget embedding (three panels present, central widgets are
//          children of the tab container and visible after show(), tabs
//          reorderable).
//   * T3 — persistence across restarts (two instances in sequence: instance 1
//          reorders and saves, instance 2 restores the same layout).
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QTabBar>
#include <QTabWidget>

#include "ui/control_panel.hpp"
#include "ui/dashboard.hpp"
#include "ui/demo_main_window.hpp"
#include "ui/roadmap_tab.hpp"
#include "ui/schedule_editor.hpp"

namespace {

const char kOrgName[] = "PatchOrchestratorTest";
const char kAppName[] = "PatchOrchestrator";

}  // namespace

class A2LayoutTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void t1_layoutRoundTrip();
    void t2_widgetEmbedding();
    void t3_persistenceAcrossRestarts();
};

void A2LayoutTests::initTestCase()
{
    // Isolate test settings from any real user settings.
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void A2LayoutTests::init()
{
    // Start each test from a clean settings store.
    QSettings settings;
    settings.clear();
    settings.sync();
}

void A2LayoutTests::t1_layoutRoundTrip()
{
    DemoMainWindow w1;
    QCOMPARE(w1.tabWidget()->count(), 4);

    // Reorder: move Control Panel (index 2) to the front.
    w1.tabWidget()->tabBar()->moveTab(2, 0);
    QCOMPARE(w1.tabWidget()->tabText(0), QStringLiteral("Control Panel"));

    // Saving produces a non-empty state byte array.
    const QByteArray state1 = w1.saveLayout();
    QVERIFY(!state1.isEmpty());

    // A second window restores the same tab order.
    DemoMainWindow w2;
    w2.restoreLayout();
    QCOMPARE(w2.tabWidget()->tabText(0), QStringLiteral("Control Panel"));
    QCOMPARE(w2.tabWidget()->tabText(1), QStringLiteral("Dashboard"));
    QCOMPARE(w2.tabWidget()->tabText(2), QStringLiteral("Schedule Editor"));

    // Round-trip is idempotent: save -> restore -> save yields the same state.
    const QByteArray state2 = w2.saveLayout();
    QCOMPARE(state1, state2);
}

void A2LayoutTests::t2_widgetEmbedding()
{
    DemoMainWindow w;

    // The three embedded panels are exposed.
    QVERIFY(w.dashboard() != nullptr);
    QVERIFY(w.scheduleEditor() != nullptr);
    QVERIFY(w.controlPanel() != nullptr);
    QVERIFY(w.roadmap() != nullptr);
    QVERIFY(w.tabWidget() != nullptr);
    QCOMPARE(w.tabWidget()->count(), 4);

    // Each embedded panel window is hosted inside the tab container (its
    // direct parent is the tab widget's internal stack), and its central
    // widget is a child of the panel (so the panel owns it).
    QVERIFY(w.tabWidget()->isAncestorOf(w.dashboard()));
    QVERIFY(w.tabWidget()->isAncestorOf(w.scheduleEditor()));
    QVERIFY(w.tabWidget()->isAncestorOf(w.controlPanel()));
    QVERIFY(w.tabWidget()->isAncestorOf(w.roadmap()));
    QCOMPARE(w.tabWidget()->indexOf(w.dashboard()), 0);
    QCOMPARE(w.tabWidget()->indexOf(w.scheduleEditor()), 1);
    QCOMPARE(w.tabWidget()->indexOf(w.controlPanel()), 2);
    QCOMPARE(w.tabWidget()->indexOf(w.roadmap()), 3);
    QVERIFY(w.dashboard()->centralWidget() != nullptr);
    QVERIFY(w.scheduleEditor()->centralWidget() != nullptr);
    QVERIFY(w.controlPanel()->centralWidget() != nullptr);

    // After show(), the current tab is visible; switching tabs makes each
    // panel visible in turn.
    w.show();
    QVERIFY(w.dashboard()->isVisible());
    w.tabWidget()->setCurrentIndex(1);
    QVERIFY(w.scheduleEditor()->isVisible());
    w.tabWidget()->setCurrentIndex(2);
    QVERIFY(w.controlPanel()->isVisible());

    // Reordering a tab changes its index.
    const int before = w.tabWidget()->indexOf(w.controlPanel());
    QCOMPARE(before, 2);
    w.tabWidget()->tabBar()->moveTab(2, 0);
    QCOMPARE(w.tabWidget()->indexOf(w.controlPanel()), 0);
}

void A2LayoutTests::t3_persistenceAcrossRestarts()
{
    // Instance 1: reorder (move Schedule Editor to the end) and save.
    DemoMainWindow w1;
    w1.tabWidget()->tabBar()->moveTab(1, 2);
    w1.saveLayout();

    // Instance 2: loads the settings file and restores the same layout.
    DemoMainWindow w2;
    w2.restoreLayout();
    QCOMPARE(w2.tabWidget()->tabText(0), QStringLiteral("Dashboard"));
    QCOMPARE(w2.tabWidget()->tabText(1), QStringLiteral("Control Panel"));
    QCOMPARE(w2.tabWidget()->tabText(2), QStringLiteral("Schedule Editor"));
}

QTEST_MAIN(A2LayoutTests)
#include "a2_layout_tests.moc"
