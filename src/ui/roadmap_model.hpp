// PatchOrchestrator — Sprint 8 (A8) Roadmap content model.
//
// A self-contained, UI-free loader/parser that turns a declarative roadmap
// source (a JSON document, or a JSON file on disk) into the QVector<RoadmapItem>
// contract consumed by RoadmapTab (A7). The model validates the source and
// rejects malformed input with a clear, actionable error — never crashing.
//
// The model has no Qt Widgets / Network dependency, so it is fully testable
// offscreen and without a display.

#ifndef PATCHORCHESTRATOR_UI_ROADMAP_MODEL_HPP
#define PATCHORCHESTRATOR_UI_ROADMAP_MODEL_HPP

#include <QString>
#include <QVector>

#include "ui/roadmap_tab.hpp"  // for RoadmapItem

// Loads roadmap items (title, description, status, target phase) from a
// declarative source and produces the QVector<RoadmapItem> that RoadmapTab
// renders via setItems().
class RoadmapModel
{
public:
    // Result of a load/parse: ok is true only when the whole source is valid.
    // On failure, error is a clear, human-readable message identifying the
    // problem (top-level shape, missing field, wrong type, etc.).
    struct Result
    {
        bool ok = false;
        QString error;
        QVector<RoadmapItem> items;
    };

    // Parse a JSON document (a QString containing the raw JSON text).
    static Result parse(const QString &json);

    // Read a JSON file from disk and parse it. Returns ok=false with a clear
    // error if the file cannot be opened or the content is invalid.
    static Result loadFile(const QString &path);

    // The built-in default roadmap (persistence, auth, real fleet integration,
    // observability, multi-tenant). Parsed from the same declarative JSON
    // format so the demo hub can populate the tab from the model even when no
    // external file is present.
    static Result defaultItems();
};

#endif // PATCHORCHESTRATOR_UI_ROADMAP_MODEL_HPP
