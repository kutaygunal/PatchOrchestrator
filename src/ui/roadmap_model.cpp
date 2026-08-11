// PatchOrchestrator — Sprint 8 (A8) Roadmap content model.
//
// See roadmap_model.hpp for the contract. This implementation validates a
// JSON roadmap source (top-level object with an "items" array; each item an
// object with required string fields title, description, status, targetPhase)
// and produces a QVector<RoadmapItem>. Every failure path returns a clear
// error message and never throws or crashes on malformed input.

#include "ui/roadmap_model.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

namespace {

// The built-in default roadmap, expressed in the same declarative JSON format
// the parser accepts. Kept as a string constant so the demo hub can populate
// the tab from the model without requiring an external file at runtime.
const char kDefaultRoadmapJson[] = R"json({
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

RoadmapModel::Result RoadmapModel::parse(const QString &json)
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
        result.error = QStringLiteral("Invalid roadmap: top-level value must be a JSON object.");
        return result;
    }
    const QJsonObject root = doc.object();

    // 3. Required "items" array.
    if (!root.contains(QLatin1String("items"))) {
        result.error = QStringLiteral("Invalid roadmap: missing required field \"items\".");
        return result;
    }
    const QJsonValue itemsValue = root.value(QLatin1String("items"));
    if (!itemsValue.isArray()) {
        result.error = QStringLiteral("Invalid roadmap: field \"items\" must be a JSON array.");
        return result;
    }
    const QJsonArray items = itemsValue.toArray();

    // 4. Validate each item. An empty array is valid (yields an empty model).
    for (int i = 0; i < items.size(); ++i) {
        const QJsonValue itemValue = items.at(i);
        if (!itemValue.isObject()) {
            result.error = QStringLiteral("Invalid roadmap: item %1 must be a JSON object.").arg(i);
            return result;
        }
        const QJsonObject item = itemValue.toObject();

        // Each required field must be present and a non-empty string.
        const QStringList required = {QStringLiteral("title"),
                                      QStringLiteral("description"),
                                      QStringLiteral("status"),
                                      QStringLiteral("targetPhase")};
        for (const QString &field : required) {
            if (!item.contains(field)) {
                result.error = QStringLiteral("Invalid roadmap: item %1 is missing required field \"%2\".")
                                   .arg(i)
                                   .arg(field);
                return result;
            }
            const QJsonValue value = item.value(field);
            if (!value.isString()) {
                result.error = QStringLiteral("Invalid roadmap: item %1 field \"%2\" must be a string.")
                                   .arg(i)
                                   .arg(field);
                return result;
            }
            if (value.toString().isEmpty()) {
                result.error = QStringLiteral("Invalid roadmap: item %1 field \"%2\" must not be empty.")
                                   .arg(i)
                                   .arg(field);
                return result;
            }
        }

        RoadmapItem parsed;
        parsed.title = item.value(QLatin1String("title")).toString();
        parsed.description = item.value(QLatin1String("description")).toString();
        parsed.status = item.value(QLatin1String("status")).toString();
        parsed.targetPhase = item.value(QLatin1String("targetPhase")).toString();
        result.items.push_back(parsed);
    }

    result.ok = true;
    return result;
}

RoadmapModel::Result RoadmapModel::loadFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Result result;
        result.error = QStringLiteral("Cannot open roadmap file \"%1\": %2")
                           .arg(path, file.errorString());
        return result;
    }
    const QString content = QString::fromUtf8(file.readAll());
    return parse(content);
}

RoadmapModel::Result RoadmapModel::defaultItems()
{
    return parse(QString::fromLatin1(kDefaultRoadmapJson));
}
