// PatchOrchestrator — Sprint 7 (A7) Roadmap/Future tab tests (Qt Test).
//
// Covers RoadmapTab, the widget that renders the project's future roadmap
// items (persistence, auth, real fleet integration, observability,
// multi-tenant) as a styled, scrollable view:
//   * T1 — renders items from the model (one entry per item, all fields).
//   * T2 — scrollable: a scroll area is present and content can be scrolled.
//   * T3 — styled: the view applies a stylesheet and stays readable.
//   * T4 — empty model renders a placeholder without crashing.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>

#include "ui/roadmap_tab.hpp"

namespace {

QVector<RoadmapItem> makeItems()
{
    QVector<RoadmapItem> items;
    items.push_back({QStringLiteral("Persistence"),
                     QStringLiteral("Persist state across restarts."),
                     QStringLiteral("Planned"), QStringLiteral("Phase B")});
    items.push_back({QStringLiteral("Authentication"),
                     QStringLiteral("Add role-based access control."),
                     QStringLiteral("Planned"), QStringLiteral("Phase B")});
    items.push_back({QStringLiteral("Real Fleet Integration"),
                     QStringLiteral("Connect to a live fleet."),
                     QStringLiteral("In progress"), QStringLiteral("Phase B")});
    items.push_back({QStringLiteral("Observability"),
                     QStringLiteral("Stream live status and metrics."),
                     QStringLiteral("Planned"), QStringLiteral("Phase C")});
    items.push_back({QStringLiteral("Multi-Tenant Support"),
                     QStringLiteral("Isolate fleets per tenant."),
                     QStringLiteral("Backlog"), QStringLiteral("Phase D")});
    return items;
}

}  // namespace

class A7RoadmapTabTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_rendersItems();
    void t2_scrollable();
    void t3_styled();
    void t4_emptyModel();
};

void A7RoadmapTabTests::t1_rendersItems()
{
    const QVector<RoadmapItem> items = makeItems();

    RoadmapTab tab;
    tab.setItems(items);

    // One entry per roadmap item.
    QCOMPARE(tab.itemCount(), items.size());

    // Each entry exposes its title, description, status, and target phase.
    for (int i = 0; i < items.size(); ++i) {
        QCOMPARE(tab.itemTitle(i), items.at(i).title);
        QCOMPARE(tab.itemDescription(i), items.at(i).description);
        QCOMPARE(tab.itemStatus(i), items.at(i).status);
        QCOMPARE(tab.itemTargetPhase(i), items.at(i).targetPhase);
    }

    // The rendered count matches the model's item count.
    QCOMPARE(tab.itemCount(), 5);
}

void A7RoadmapTabTests::t2_scrollable()
{
    RoadmapTab tab;
    tab.setItems(makeItems());

    // A scroll area is present.
    QVERIFY(tab.scrollArea() != nullptr);
    QVERIFY(tab.scrollArea()->isWidgetType());

    // With enough items to overflow the viewport, scrolling is enabled and
    // content can be scrolled into view.
    tab.resize(400, 200);
    tab.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tab));

    QScrollBar *vbar = tab.scrollArea()->verticalScrollBar();
    QVERIFY(vbar != nullptr);
    QVERIFY(vbar->maximum() > 0);

    // Scrolling moves the viewport.
    vbar->setValue(vbar->maximum());
    QCOMPARE(vbar->value(), vbar->maximum());
}

void A7RoadmapTabTests::t3_styled()
{
    RoadmapTab tab;
    tab.setItems(makeItems());

    // The tab applies a stylesheet to the roadmap view.
    QVERIFY(!tab.styleSheet().isEmpty());

    // Styling does not break rendering; all items remain present/readable.
    QCOMPARE(tab.itemCount(), makeItems().size());
    for (int i = 0; i < tab.itemCount(); ++i) {
        QVERIFY(!tab.itemTitle(i).isEmpty());
        QVERIFY(!tab.itemDescription(i).isEmpty());
        QVERIFY(!tab.itemStatus(i).isEmpty());
        QVERIFY(!tab.itemTargetPhase(i).isEmpty());
    }
}

void A7RoadmapTabTests::t4_emptyModel()
{
    RoadmapTab tab;
    tab.setItems({});

    // An empty model renders without crashing and shows a placeholder.
    QCOMPARE(tab.itemCount(), 0);
    QVERIFY(tab.emptyLabel() != nullptr);

    // Show the tab so isVisible() reflects the on-screen state.
    tab.resize(400, 200);
    tab.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tab));
    QVERIFY(tab.emptyLabel()->isVisible());
    QVERIFY(!tab.emptyLabel()->text().isEmpty());
}

QTEST_MAIN(A7RoadmapTabTests)
#include "a7_roadmap_tab_tests.moc"
