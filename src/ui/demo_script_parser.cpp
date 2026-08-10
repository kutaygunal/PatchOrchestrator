// PatchOrchestrator — Sprint 5 (A5) Declarative demo script parser.
//
// See demo_script_parser.hpp for the contract. This implementation validates a
// JSON demo script against a schema (top-level object with a non-empty "steps"
// array; each step an object with a recognized "type") and produces an ordered
// list of DemoScriptStep. Every failure path returns a clear error message and
// never throws or crashes on malformed input.

#include "ui/demo_script_parser.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

namespace {

// Map a recognized type-name string to its enum. Returns false if unknown.
bool typeFromName(const QString &name, DemoStepType *out)
{
    if (name == QLatin1String("load_scenario")) {
        *out = DemoStepType::LoadScenario;
        return true;
    }
    if (name == QLatin1String("schedule")) {
        *out = DemoStepType::Schedule;
        return true;
    }
    if (name == QLatin1String("simulate")) {
        *out = DemoStepType::Simulate;
        return true;
    }
    if (name == QLatin1String("pause")) {
        *out = DemoStepType::Pause;
        return true;
    }
    if (name == QLatin1String("resume")) {
        *out = DemoStepType::Resume;
        return true;
    }
    if (name == QLatin1String("rollback")) {
        *out = DemoStepType::Rollback;
        return true;
    }
    if (name == QLatin1String("show_roadmap")) {
        *out = DemoStepType::ShowRoadmap;
        return true;
    }
    return false;
}

}  // namespace

QString DemoScriptParser::typeName(DemoStepType type)
{
    switch (type) {
    case DemoStepType::LoadScenario: return QStringLiteral("load_scenario");
    case DemoStepType::Schedule:     return QStringLiteral("schedule");
    case DemoStepType::Simulate:     return QStringLiteral("simulate");
    case DemoStepType::Pause:        return QStringLiteral("pause");
    case DemoStepType::Resume:       return QStringLiteral("resume");
    case DemoStepType::Rollback:     return QStringLiteral("rollback");
    case DemoStepType::ShowRoadmap:  return QStringLiteral("show_roadmap");
    }
    return QString();
}

QStringList DemoScriptParser::recognizedTypeNames()
{
    return {QStringLiteral("load_scenario"), QStringLiteral("schedule"),
            QStringLiteral("simulate"),      QStringLiteral("pause"),
            QStringLiteral("resume"),        QStringLiteral("rollback"),
            QStringLiteral("show_roadmap")};
}

DemoScriptParser::Result DemoScriptParser::parse(const QString &json)
{
    Result result;

    // 1. Syntactic JSON validation.
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        result.error = QStringLiteral("Invalid JSON: %1 (at offset %2)")
                           .arg(parseError.errorString())
                           .arg(parseError.offset);
        return result;
    }

    // 2. Top-level shape: must be an object.
    if (!doc.isObject()) {
        result.error = QStringLiteral("Invalid script: top-level value must be a JSON object.");
        return result;
    }
    const QJsonObject root = doc.object();

    // 3. Required "steps" array.
    if (!root.contains(QLatin1String("steps"))) {
        result.error = QStringLiteral("Invalid script: missing required field \"steps\".");
        return result;
    }
    const QJsonValue stepsValue = root.value(QLatin1String("steps"));
    if (!stepsValue.isArray()) {
        result.error = QStringLiteral("Invalid script: field \"steps\" must be a JSON array.");
        return result;
    }
    const QJsonArray steps = stepsValue.toArray();

    // 4. Non-empty step list.
    if (steps.isEmpty()) {
        result.error = QStringLiteral("Invalid script: \"steps\" must contain at least one step.");
        return result;
    }

    // 5. Validate each step.
    for (int i = 0; i < steps.size(); ++i) {
        const QJsonValue stepValue = steps.at(i);
        if (!stepValue.isObject()) {
            result.error = QStringLiteral("Invalid script: step %1 must be a JSON object.").arg(i);
            return result;
        }
        const QJsonObject step = stepValue.toObject();

        // 5a. Required "type" field, must be a string.
        if (!step.contains(QLatin1String("type"))) {
            result.error = QStringLiteral("Invalid script: step %1 is missing required field \"type\".").arg(i);
            return result;
        }
        const QJsonValue typeValue = step.value(QLatin1String("type"));
        if (!typeValue.isString()) {
            result.error = QStringLiteral("Invalid script: step %1 field \"type\" must be a string.").arg(i);
            return result;
        }
        const QString typeName = typeValue.toString();

        // 5b. Recognized step type.
        DemoStepType type;
        if (!typeFromName(typeName, &type)) {
            result.error = QStringLiteral("Invalid script: step %1 has unknown type \"%2\". "
                                          "Expected one of: %3.")
                               .arg(i)
                               .arg(typeName, recognizedTypeNames().join(QStringLiteral(", ")));
            return result;
        }

        // 5c. Optional "id" and "narration" must be strings if present.
        if (step.contains(QLatin1String("id")) && !step.value(QLatin1String("id")).isString()) {
            result.error = QStringLiteral("Invalid script: step %1 field \"id\" must be a string.").arg(i);
            return result;
        }
        if (step.contains(QLatin1String("narration")) &&
            !step.value(QLatin1String("narration")).isString()) {
            result.error = QStringLiteral("Invalid script: step %1 field \"narration\" must be a string.").arg(i);
            return result;
        }

        // 5d. Build the parsed step, preserving type-specific fields verbatim.
        DemoScriptStep parsed;
        parsed.type = type;
        parsed.id = step.value(QLatin1String("id")).toString(typeName);
        parsed.narration = step.value(QLatin1String("narration")).toString();
        for (auto it = step.constBegin(); it != step.constEnd(); ++it) {
            if (it.key() == QLatin1String("type") ||
                it.key() == QLatin1String("id") ||
                it.key() == QLatin1String("narration"))
                continue;
            parsed.fields.insert(it.key(), it.value().toVariant());
        }
        result.steps.push_back(parsed);
    }

    result.ok = true;
    return result;
}

QVector<DemoStep> DemoScriptParser::toControllerSteps(const QVector<DemoScriptStep> &steps)
{
    QVector<DemoStep> out;
    out.reserve(steps.size());
    for (const DemoScriptStep &s : steps)
        out.push_back({s.id, s.narration});
    return out;
}
