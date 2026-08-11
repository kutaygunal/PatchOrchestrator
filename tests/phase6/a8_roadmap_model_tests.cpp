// PatchOrchestrator — Sprint 8 (A8) Roadmap content model tests (Qt Test).
//
// Covers the A8 content model that loads roadmap items (persistence, auth,
// real fleet integration, observability, multi-tenant) from a declarative
// JSON source and produces the QVector<RoadmapItem> contract consumed by
// RoadmapTab (A7):
//   * T1 — model parses/loads (item count matches source; empty source valid).
//   * T2 — field access correct (title/description/status/targetPhase mapped).
//   * T3 — A7 renders all items (RoadmapTab::setItems from the model).
//   * T4 — invalid source rejected with a clear error, without crashing.
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/roadmap_model.hpp"
#include "ui/roadmap_tab.hpp"

namespace {

// A valid declarative roadmap source with the five canonical items.
const char kValidJson[] = R"json({
  "items": [
    {
      "title": "Persistence",
      "description": "Persist schedules, rollout state, and layout across restarts.",
      "status": "Planned",
      "targetPhase": "Phase B"
    },
    {
      "title": "Authentication",
      "description": "Add role-based access control for operator actions.",
      "status": "Planned",
      "targetPhase": "Phase B"
    },
    {
      "title": "Real Fleet Integration",
      "description": "Connect the control plane to a live fleet via the engine bridge.",
      "status": "In progress",
      "targetPhase": "Phase B"
    },
    {
      "title": "Observability",
      "description": "Stream live status and expose metrics for monitoring.",
      "status": "Planned",
      "targetPhase": "Phase C"
    },
    {
      "title": "Multi-Tenant Support",
      "description": "Isolate fleets and schedules per tenant.",
      "status": "Backlog",
      "targetPhase": "Phase D"
    }
  ]
})json";

}  // namespace

class A8RoadmapModelTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_parsesLoads();
    void t2_fieldAccess();
    void t3_rendersAllItems();
    void t4_invalidRejected();
};

void A8RoadmapModelTests::t1_parsesLoads()
{
    // The model loads/parses the source without error.
    const RoadmapModel::Result result = RoadmapModel::parse(QString::fromLatin1(kValidJson));
    QVERIFY(result.ok);
    QVERIFY(result.error.isEmpty());

    // The resulting item count matches the source (5 items).
    QCOMPARE(result.items.size(), 5);

    // The built-in default source also parses to the same item count.
    const RoadmapModel::Result def = RoadmapModel::defaultItems();
    QVERIFY(def.ok);
    QCOMPARE(def.items.size(), 5);

    // Loading an empty source yields a valid, empty model without crashing.
    const RoadmapModel::Result empty =
        RoadmapModel::parse(QStringLiteral("{\"items\": []}"));
    QVERIFY(empty.ok);
    QVERIFY(empty.items.isEmpty());
}

void A8RoadmapModelTests::t2_fieldAccess()
{
    const RoadmapModel::Result result = RoadmapModel::parse(QString::fromLatin1(kValidJson));
    QVERIFY(result.ok);

    // Each loaded item has the correct title, description, status, and
    // targetPhase matching the source (no swapped/missing fields).
    QCOMPARE(result.items.at(0).title, QStringLiteral("Persistence"));
    QCOMPARE(result.items.at(0).description,
             QStringLiteral("Persist schedules, rollout state, and layout across restarts."));
    QCOMPARE(result.items.at(0).status, QStringLiteral("Planned"));
    QCOMPARE(result.items.at(0).targetPhase, QStringLiteral("Phase B"));

    QCOMPARE(result.items.at(1).title, QStringLiteral("Authentication"));
    QCOMPARE(result.items.at(1).status, QStringLiteral("Planned"));
    QCOMPARE(result.items.at(1).targetPhase, QStringLiteral("Phase B"));

    QCOMPARE(result.items.at(2).title, QStringLiteral("Real Fleet Integration"));
    QCOMPARE(result.items.at(2).status, QStringLiteral("In progress"));
    QCOMPARE(result.items.at(2).targetPhase, QStringLiteral("Phase B"));

    QCOMPARE(result.items.at(3).title, QStringLiteral("Observability"));
    QCOMPARE(result.items.at(3).status, QStringLiteral("Planned"));
    QCOMPARE(result.items.at(3).targetPhase, QStringLiteral("Phase C"));

    QCOMPARE(result.items.at(4).title, QStringLiteral("Multi-Tenant Support"));
    QCOMPARE(result.items.at(4).status, QStringLiteral("Backlog"));
    QCOMPARE(result.items.at(4).targetPhase, QStringLiteral("Phase D"));

    // All field values are non-empty.
    for (const RoadmapItem &item : result.items) {
        QVERIFY(!item.title.isEmpty());
        QVERIFY(!item.description.isEmpty());
        QVERIFY(!item.status.isEmpty());
        QVERIFY(!item.targetPhase.isEmpty());
    }
}

void A8RoadmapModelTests::t3_rendersAllItems()
{
    // Load the model and pass the resulting items to RoadmapTab via setItems().
    const RoadmapModel::Result result = RoadmapModel::parse(QString::fromLatin1(kValidJson));
    QVERIFY(result.ok);

    RoadmapTab tab;
    tab.setItems(result.items);

    // RoadmapTab::itemCount() equals the model's item count.
    QCOMPARE(tab.itemCount(), result.items.size());

    // The accessors return the model's values for each index (all rendered).
    for (int i = 0; i < result.items.size(); ++i) {
        QCOMPARE(tab.itemTitle(i), result.items.at(i).title);
        QCOMPARE(tab.itemDescription(i), result.items.at(i).description);
        QCOMPARE(tab.itemStatus(i), result.items.at(i).status);
        QCOMPARE(tab.itemTargetPhase(i), result.items.at(i).targetPhase);
    }

    // No items dropped: every model item is rendered.
    QCOMPARE(tab.itemCount(), 5);
}

void A8RoadmapModelTests::t4_invalidRejected()
{
    // Malformed JSON is rejected with a clear error, without crashing.
    RoadmapModel::Result bad = RoadmapModel::parse(QStringLiteral("{not json"));
    QVERIFY(!bad.ok);
    QVERIFY(!bad.error.isEmpty());
    QVERIFY(bad.items.isEmpty());

    // Top-level must be an object.
    bad = RoadmapModel::parse(QStringLiteral("[1,2,3]"));
    QVERIFY(!bad.ok);
    QVERIFY(!bad.error.isEmpty());

    // Missing required "items" field.
    bad = RoadmapModel::parse(QStringLiteral("{\"foo\": 1}"));
    QVERIFY(!bad.ok);
    QVERIFY(!bad.error.isEmpty());

    // "items" must be an array.
    bad = RoadmapModel::parse(QStringLiteral("{\"items\": 42}"));
    QVERIFY(!bad.ok);
    QVERIFY(!bad.error.isEmpty());

    // Item missing a required field (error names the missing field).
    bad = RoadmapModel::parse(QStringLiteral(
        "{\"items\": [{\"title\": \"X\", \"description\": \"d\", \"status\": \"s\"}]}"));
    QVERIFY(!bad.ok);
    QVERIFY(bad.error.contains(QLatin1String("targetPhase")));

    // Item field of the wrong type.
    bad = RoadmapModel::parse(QStringLiteral(
        "{\"items\": [{\"title\": 123, \"description\": \"d\", \"status\": \"s\", "
        "\"targetPhase\": \"P\"}]}"));
    QVERIFY(!bad.ok);
    QVERIFY(!bad.error.isEmpty());

    // Item with an empty required field.
    bad = RoadmapModel::parse(QStringLiteral(
        "{\"items\": [{\"title\": \"\", \"description\": \"d\", \"status\": \"s\", "
        "\"targetPhase\": \"P\"}]}"));
    QVERIFY(!bad.ok);
    QVERIFY(!bad.error.isEmpty());

    // A missing file is reported as an error, not a crash.
    bad = RoadmapModel::loadFile(QStringLiteral("does_not_exist_roadmap.json"));
    QVERIFY(!bad.ok);
    QVERIFY(!bad.error.isEmpty());
}

QTEST_MAIN(A8RoadmapModelTests)
#include "a8_roadmap_model_tests.moc"
