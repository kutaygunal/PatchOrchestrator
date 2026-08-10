// PatchOrchestrator — Sprint 5 (A5) Demo script parser tests (Qt Test).
//
// Covers the declarative JSON demo script format and its parser:
//   * T1 — valid JSON parses (all step types, correct count, fields populated).
//   * T2 — all step types recognized; unknown types rejected.
//   * T3 — invalid JSON rejected with a clear error (no crash).
//   * T4 — schema validation (missing/extra/wrong-typed fields).
//
// Runs offscreen (QT_QPA_PLATFORM=offscreen) so no display is required.

#include <QtTest/QtTest>

#include "ui/demo_script_parser.hpp"

namespace {

// A valid script containing every supported step type.
const char *kValidScript = R"json({
  "name": "Full walkthrough",
  "steps": [
    {"type": "load_scenario", "id": "load", "narration": "Load a scenario.",
     "scenario": "small_clean_fleet"},
    {"type": "schedule", "id": "sched", "narration": "Create a schedule.",
     "target": "schedule-1"},
    {"type": "simulate", "id": "sim", "narration": "Simulate a rollout.",
     "parameters": {"fleet_size": 10, "failure_rate": 0.1}},
    {"type": "pause", "id": "pause", "narration": "Pause here."},
    {"type": "resume", "id": "resume", "narration": "Resume now."},
    {"type": "rollback", "id": "rb", "narration": "Roll back.",
     "target": "schedule-1"},
    {"type": "show_roadmap", "id": "roadmap", "narration": "Show the roadmap."}
  ]
})json";

}  // namespace

class A5ScriptParserTests : public QObject
{
    Q_OBJECT

private slots:
    void t1_validParse();
    void t2_stepTypes();
    void t3_invalidReject();
    void t4_schemaValidation();
};

void A5ScriptParserTests::t1_validParse()
{
    const auto result = DemoScriptParser::parse(QString::fromUtf8(kValidScript));

    // The parser accepts the valid JSON without error.
    QVERIFY(result.ok);
    QVERIFY(result.error.isEmpty());

    // The parsed step list has the expected number of steps.
    QCOMPARE(result.steps.size(), 7);

    // Each step's type is recognized and mapped correctly.
    QCOMPARE(result.steps.at(0).type, DemoStepType::LoadScenario);
    QCOMPARE(result.steps.at(1).type, DemoStepType::Schedule);
    QCOMPARE(result.steps.at(2).type, DemoStepType::Simulate);
    QCOMPARE(result.steps.at(3).type, DemoStepType::Pause);
    QCOMPARE(result.steps.at(4).type, DemoStepType::Resume);
    QCOMPARE(result.steps.at(5).type, DemoStepType::Rollback);
    QCOMPARE(result.steps.at(6).type, DemoStepType::ShowRoadmap);

    // Step fields (id, narration, type-specific fields) are populated.
    QCOMPARE(result.steps.at(0).id, QStringLiteral("load"));
    QCOMPARE(result.steps.at(0).narration, QStringLiteral("Load a scenario."));
    QCOMPARE(result.steps.at(0).fields.value(QStringLiteral("scenario")).toString(),
             QStringLiteral("small_clean_fleet"));

    QCOMPARE(result.steps.at(2).id, QStringLiteral("sim"));
    QCOMPARE(result.steps.at(2).fields.value(QStringLiteral("parameters")).toMap()
                 .value(QStringLiteral("fleet_size")).toInt(), 10);
    QCOMPARE(result.steps.at(2).fields.value(QStringLiteral("parameters")).toMap()
                 .value(QStringLiteral("failure_rate")).toDouble(), 0.1);

    QCOMPARE(result.steps.at(6).id, QStringLiteral("roadmap"));

    // The parsed steps convert cleanly to the controller's DemoStep list.
    const auto controllerSteps = DemoScriptParser::toControllerSteps(result.steps);
    QCOMPARE(controllerSteps.size(), 7);
    QCOMPARE(controllerSteps.at(0).id, QStringLiteral("load"));
    QCOMPARE(controllerSteps.at(0).narration, QStringLiteral("Load a scenario."));
}

void A5ScriptParserTests::t2_stepTypes()
{
    // Each of the seven step types is recognized.
    const QStringList types = DemoScriptParser::recognizedTypeNames();
    QCOMPARE(types.size(), 7);
    QVERIFY(types.contains(QStringLiteral("load_scenario")));
    QVERIFY(types.contains(QStringLiteral("schedule")));
    QVERIFY(types.contains(QStringLiteral("simulate")));
    QVERIFY(types.contains(QStringLiteral("pause")));
    QVERIFY(types.contains(QStringLiteral("resume")));
    QVERIFY(types.contains(QStringLiteral("rollback")));
    QVERIFY(types.contains(QStringLiteral("show_roadmap")));

    // typeName round-trips each enum value.
    QCOMPARE(DemoScriptParser::typeName(DemoStepType::LoadScenario), QStringLiteral("load_scenario"));
    QCOMPARE(DemoScriptParser::typeName(DemoStepType::Schedule), QStringLiteral("schedule"));
    QCOMPARE(DemoScriptParser::typeName(DemoStepType::Simulate), QStringLiteral("simulate"));
    QCOMPARE(DemoScriptParser::typeName(DemoStepType::Pause), QStringLiteral("pause"));
    QCOMPARE(DemoScriptParser::typeName(DemoStepType::Resume), QStringLiteral("resume"));
    QCOMPARE(DemoScriptParser::typeName(DemoStepType::Rollback), QStringLiteral("rollback"));
    QCOMPARE(DemoScriptParser::typeName(DemoStepType::ShowRoadmap), QStringLiteral("show_roadmap"));

    // An unknown step type is rejected (not silently ignored).
    const QString bad = QStringLiteral(R"({"steps":[{"type":"teleport"}]})");
    const auto result = DemoScriptParser::parse(bad);
    QVERIFY(!result.ok);
    QVERIFY(result.error.contains(QStringLiteral("unknown type")));
    QVERIFY(result.error.contains(QStringLiteral("teleport")));
}

void A5ScriptParserTests::t3_invalidReject()
{
    // Syntactically invalid JSON.
    {
        const auto r = DemoScriptParser::parse(QStringLiteral("{ not json"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("Invalid JSON")));
    }

    // Top-level not an object.
    {
        const auto r = DemoScriptParser::parse(QStringLiteral("[1,2,3]"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("object")));
    }

    // Missing required "steps" field.
    {
        const auto r = DemoScriptParser::parse(QStringLiteral(R"({"name":"x"})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("steps")));
    }

    // "steps" not an array.
    {
        const auto r = DemoScriptParser::parse(QStringLiteral(R"({"steps": 42})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("array")));
    }

    // Empty step list.
    {
        const auto r = DemoScriptParser::parse(QStringLiteral(R"({"steps":[]})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("at least one")));
    }

    // Step not an object.
    {
        const auto r = DemoScriptParser::parse(QStringLiteral(R"({"steps":[42]})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("object")));
    }

    // Step missing "type".
    {
        const auto r = DemoScriptParser::parse(QStringLiteral(R"({"steps":[{"id":"a"}]})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("type")));
    }

    // Step "type" not a string.
    {
        const auto r = DemoScriptParser::parse(QStringLiteral(R"({"steps":[{"type":7}]})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("string")));
    }

    // Unknown step type.
    {
        const auto r = DemoScriptParser::parse(QStringLiteral(R"({"steps":[{"type":"nope"}]})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("unknown type")));
    }
}

void A5ScriptParserTests::t4_schemaValidation()
{
    // A valid script passes schema validation.
    {
        const auto r = DemoScriptParser::parse(QString::fromUtf8(kValidScript));
        QVERIFY(r.ok);
    }

    // Wrong-typed "id" fails validation with a clear message.
    {
        const auto r = DemoScriptParser::parse(
            QStringLiteral(R"({"steps":[{"type":"pause","id":123}]})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("id")));
        QVERIFY(r.error.contains(QStringLiteral("string")));
    }

    // Wrong-typed "narration" fails validation with a clear message.
    {
        const auto r = DemoScriptParser::parse(
            QStringLiteral(R"({"steps":[{"type":"pause","narration":true}]})"));
        QVERIFY(!r.ok);
        QVERIFY(r.error.contains(QStringLiteral("narration")));
        QVERIFY(r.error.contains(QStringLiteral("string")));
    }

    // Extra (unknown) fields are tolerated and preserved, not rejected.
    {
        const auto r = DemoScriptParser::parse(
            QStringLiteral(R"({"steps":[{"type":"simulate","custom":"kept"}]})"));
        QVERIFY(r.ok);
        QCOMPARE(r.steps.size(), 1);
        QCOMPARE(r.steps.at(0).fields.value(QStringLiteral("custom")).toString(),
                 QStringLiteral("kept"));
    }
}

QTEST_MAIN(A5ScriptParserTests)
#include "a5_script_parser_tests.moc"
