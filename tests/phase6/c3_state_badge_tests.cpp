// PatchOrchestrator — Sprint 20 (C3) state badge renderer tests (Qt Test).
//
// Covers the reusable StateBadge widget that renders a color-coded patch
// state with a legend:
//   * T1 (c3_badge_render) — each state renders the correct color/icon/label
//          and the badge reflects the state it is given.
//   * T2 (c3_reusable) — the badge can be constructed standalone and embedded
//          in other widgets; multiple badges coexist with different states;
//          updating a badge's state re-renders it correctly.
//   * T3 (c3_legend) — the legend explains the color coding and state
//          meanings; all six states appear with their color and meaning.
//   * T4 (c3_regression) — the dashboard still renders endpoint data using the
//          badge and the existing P8/C1/C2 behavior is preserved.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/dashboard.hpp"
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

class C3StateBadgeTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void t1_badgeRender();
    void t2_reusable();
    void t3_legend();
    void t4_regression();
};

void C3StateBadgeTests::initTestCase()
{
    QCoreApplication::setOrganizationName(QLatin1String(kOrgName));
    QCoreApplication::setApplicationName(QLatin1String(kAppName));
}

void C3StateBadgeTests::t1_badgeRender()
{
    // Each state renders the correct color (from the C2 mapping).
    QCOMPARE(StateBadge::colorForState(QStringLiteral("succeeded")),
             QColor(0x2e, 0x7d, 0x32));  // green
    QCOMPARE(StateBadge::colorForState(QStringLiteral("failed")),
             QColor(0xc6, 0x28, 0x28));  // red
    QCOMPARE(StateBadge::colorForState(QStringLiteral("paused")),
             QColor(0xf9, 0xa8, 0x25));  // amber
    QCOMPARE(StateBadge::colorForState(QStringLiteral("running")),
             QColor(0x15, 0x65, 0xc0));  // blue
    QCOMPARE(StateBadge::colorForState(QStringLiteral("pending")),
             QColor(0x9e, 0x9e, 0x9e));  // grey
    QCOMPARE(StateBadge::colorForState(QStringLiteral("rolled_back")),
             QColor(0x6a, 0x1b, 0x9a));  // purple

    // Unknown/empty states map to a defined default color (no crash).
    const QColor defaultColor = StateBadge::colorForState(QStringLiteral("unknown"));
    QVERIFY(defaultColor.isValid());
    QCOMPARE(defaultColor, StateBadge::colorForState(QString()));
    QCOMPARE(defaultColor, StateBadge::colorForState(QStringLiteral("bogus_state")));

    // The badge reflects the state it is given: color, label, and icon.
    StateBadge badge(QStringLiteral("succeeded"));
    QCOMPARE(badge.state(), QStringLiteral("succeeded"));
    QCOMPARE(badge.color(), StateBadge::colorForState(QStringLiteral("succeeded")));
    QCOMPARE(badge.label(), QStringLiteral("Succeeded"));
    QVERIFY(!badge.icon().isEmpty());

    StateBadge failed(QStringLiteral("failed"));
    QCOMPARE(failed.state(), QStringLiteral("failed"));
    QCOMPARE(failed.color(), StateBadge::colorForState(QStringLiteral("failed")));
    QCOMPARE(failed.label(), QStringLiteral("Failed"));
    QVERIFY(!failed.icon().isEmpty());

    // Distinct states produce distinct colors/icons.
    QVERIFY(badge.color() != failed.color());
    QVERIFY(badge.icon() != failed.icon());
}

void C3StateBadgeTests::t2_reusable()
{
    // Constructed standalone (no parent) and embedded in another widget.
    StateBadge standalone(QStringLiteral("running"));
    QCOMPARE(standalone.state(), QStringLiteral("running"));
    QCOMPARE(standalone.color(), StateBadge::colorForState(QStringLiteral("running")));

    QWidget host;
    StateBadge embedded(QStringLiteral("pending"), &host);
    QCOMPARE(embedded.parent(), &host);
    QCOMPARE(embedded.state(), QStringLiteral("pending"));

    // Multiple badges can coexist with different states.
    StateBadge a(QStringLiteral("succeeded"));
    StateBadge b(QStringLiteral("failed"));
    StateBadge c(QStringLiteral("rolled_back"));
    QVERIFY(a.color() != b.color());
    QVERIFY(b.color() != c.color());
    QVERIFY(a.color() != c.color());

    // Updating a badge's state re-renders it correctly.
    StateBadge updatable(QStringLiteral("pending"));
    QCOMPARE(updatable.color(), StateBadge::colorForState(QStringLiteral("pending")));
    updatable.setState(QStringLiteral("succeeded"));
    QCOMPARE(updatable.state(), QStringLiteral("succeeded"));
    QCOMPARE(updatable.color(), StateBadge::colorForState(QStringLiteral("succeeded")));
    QCOMPARE(updatable.label(), QStringLiteral("Succeeded"));
}

void C3StateBadgeTests::t3_legend()
{
    // The legend explains the color coding and state meanings.
    const QString legend = StateBadge::legendText();
    QVERIFY(!legend.isEmpty());

    // All six states appear in the legend with their color and meaning.
    const QStringList states = StateBadge::legendStates();
    QCOMPARE(states.size(), 6);

    for (const QString &s : states) {
        QVERIFY2(legend.contains(StateBadge::labelForState(s)),
                 qPrintable(QStringLiteral("legend missing label for %1").arg(s)));
        QVERIFY2(legend.contains(StateBadge::colorForState(s).name()),
                 qPrintable(QStringLiteral("legend missing color for %1").arg(s)));
        QVERIFY2(legend.contains(StateBadge::iconForState(s)),
                 qPrintable(QStringLiteral("legend missing icon for %1").arg(s)));
    }

    // The six known states are exactly the expected set.
    QVERIFY(states.contains(QStringLiteral("succeeded")));
    QVERIFY(states.contains(QStringLiteral("failed")));
    QVERIFY(states.contains(QStringLiteral("paused")));
    QVERIFY(states.contains(QStringLiteral("running")));
    QVERIFY(states.contains(QStringLiteral("pending")));
    QVERIFY(states.contains(QStringLiteral("rolled_back")));
}

void C3StateBadgeTests::t4_regression()
{
    DashboardWindow w;
    w.stopPolling();  // isolate the event path from the poll timer

    // The dashboard still renders endpoint data correctly using the badge.
    w.handleStreamEvent(makeEvent(
        QStringLiteral("running"),
        {{QStringLiteral("ep-1"), QStringLiteral("succeeded")},
         {QStringLiteral("ep-2"), QStringLiteral("failed")}}));
    QCOMPARE(w.rowCount(), 2);
    QCOMPARE(w.cellText(0, 0), QStringLiteral("ep-1"));
    QCOMPARE(w.cellText(0, 1), QStringLiteral("succeeded"));
    QCOMPARE(w.cellText(1, 0), QStringLiteral("ep-2"));
    QCOMPARE(w.cellText(1, 1), QStringLiteral("failed"));

    // The state cell now hosts a reusable StateBadge widget.
    QVERIFY(w.rowStateBadge(0) != nullptr);
    QVERIFY(w.rowStateBadge(1) != nullptr);
    auto *badge0 = qobject_cast<StateBadge *>(w.rowStateBadge(0));
    auto *badge1 = qobject_cast<StateBadge *>(w.rowStateBadge(1));
    QVERIFY(badge0 != nullptr);
    QVERIFY(badge1 != nullptr);
    QCOMPARE(badge0->state(), QStringLiteral("succeeded"));
    QCOMPARE(badge1->state(), QStringLiteral("failed"));

    // C1 progress bars are still present and animating to their targets.
    QCOMPARE(w.progressBarCount(), 2);
    QCOMPARE(w.progressBarTarget(0), 50);
    QCOMPARE(w.progressBarTarget(1), 50);

    // C2 colors are still applied alongside the badge rendering.
    QCOMPARE(w.rowStateColor(0), DashboardWindow::colorForState(QStringLiteral("succeeded")));
    QCOMPARE(w.rowStateColor(1), DashboardWindow::colorForState(QStringLiteral("failed")));

    // The dashboard mapping stays in sync with the badge mapping.
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("succeeded")),
             StateBadge::colorForState(QStringLiteral("succeeded")));
    QCOMPARE(DashboardWindow::colorForState(QStringLiteral("failed")),
             StateBadge::colorForState(QStringLiteral("failed")));
}

QTEST_MAIN(C3StateBadgeTests)
#include "c3_state_badge_tests.moc"
