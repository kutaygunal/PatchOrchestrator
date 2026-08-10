// PatchOrchestrator — Sprint 5 (A5) Declarative demo script definition.
//
// A self-contained, UI-free parser that turns a declarative JSON demo script
// into the ordered step list consumed by DemoModeController (S4). The format
// supports the step types: load scenario, schedule, simulate, pause, resume,
// rollback, and show roadmap. Invalid scripts are rejected with a clear,
// actionable error message (no crash on malformed input).
//
// The parser has no Qt Widgets / Network dependency, so it is fully testable
// offscreen and without a display.

#ifndef PATCHORCHESTRATOR_UI_DEMO_SCRIPT_PARSER_HPP
#define PATCHORCHESTRATOR_UI_DEMO_SCRIPT_PARSER_HPP

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

#include "ui/demo_mode_controller.hpp"  // for DemoStep

// The seven supported demo step types.
enum class DemoStepType
{
    LoadScenario,  // load a scenario preset
    Schedule,      // create / show a schedule
    Simulate,      // run a simulated rollout
    Pause,         // pause the walkthrough
    Resume,        // resume the walkthrough
    Rollback,      // roll back a rollout
    ShowRoadmap    // show the future roadmap
};

// A single parsed demo script step: its type, a stable id, the narration text
// shown to the viewer, and any type-specific fields (e.g. scenario, target,
// parameters) preserved verbatim from the JSON.
struct DemoScriptStep
{
    DemoStepType type;
    QString id;
    QString narration;
    QVariantMap fields;
};

// Parses a declarative JSON demo script into a validated step list.
class DemoScriptParser
{
public:
    // Result of a parse: ok is true only when the whole script is valid.
    // On failure, error is a clear, human-readable message identifying the
    // problem (top-level shape, missing field, wrong type, unknown step type,
    // empty step list, etc.).
    struct Result
    {
        bool ok = false;
        QString error;
        QVector<DemoScriptStep> steps;
    };

    // Parse a JSON document (a QString containing the raw JSON text).
    static Result parse(const QString &json);

    // Human-readable name for a step type (e.g. "load_scenario").
    static QString typeName(DemoStepType type);

    // Convert parsed steps into the plain DemoStep list the controller drives.
    // The controller only needs id + narration; type-specific fields are
    // dropped here (they are available on DemoScriptStep for richer handling).
    static QVector<DemoStep> toControllerSteps(const QVector<DemoScriptStep> &steps);

    // The set of recognized step-type names (used for validation).
    static QStringList recognizedTypeNames();
};

#endif // PATCHORCHESTRATOR_UI_DEMO_SCRIPT_PARSER_HPP
