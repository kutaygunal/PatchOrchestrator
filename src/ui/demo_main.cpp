// PatchOrchestrator — Sprint 9 (A9) unified demo entry point.
//
// Constructs and shows the DemoMainWindow that hosts the dashboard, schedule
// editor, control panel, and roadmap as tabs in a single application, and
// wires the Demo Mode bar/controller (A4/A6) into the hub so the guided
// scripted walkthrough drives the shared app context (A3).
//
// The Demo Mode controller is fed a declarative demo script (A5) parsed into
// an ordered step list; the DemoModeBar (A6) drives that controller and stays
// in sync with its step indicator and narration. The controller mirrors the
// current step into the shared DemoAppContext so the rest of the hub reflects
// the walkthrough position.

#include "demo_app_context.hpp"
#include "demo_main_window.hpp"
#include "demo_mode_bar.hpp"
#include "demo_mode_controller.hpp"
#include "demo_script_parser.hpp"

#include <QApplication>
#include <QDockWidget>

namespace {

// A small built-in declarative demo script (A5 format) so the walkthrough has
// meaningful steps on first launch. It exercises the supported step types and
// ends by showing the roadmap.
const char kDemoScript[] = R"json(
{
  "steps": [
    { "type": "load_scenario", "id": "load", "narration": "Loading the demo scenario." },
    { "type": "schedule", "id": "schedule", "narration": "Creating the rollout schedule." },
    { "type": "simulate", "id": "simulate", "narration": "Simulating the fleet rollout." },
    { "type": "pause", "id": "pause", "narration": "Pausing the rollout for review." },
    { "type": "resume", "id": "resume", "narration": "Resuming the rollout." },
    { "type": "rollback", "id": "rollback", "narration": "Rolling back the rollout." },
    { "type": "show_roadmap", "id": "roadmap", "narration": "Showing the future roadmap." }
  ]
}
)json";

}  // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("PatchOrchestrator"));
    QApplication::setApplicationName(QStringLiteral("PatchOrchestrator"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    DemoMainWindow window;
    window.show();

    // --- Demo Mode wiring (Sprint 9 / A9) ---------------------------------
    // Build the controller, feed it the parsed demo script, and bind it to the
    // shared app context so the walkthrough position is mirrored into the hub.
    DemoModeController *controller = new DemoModeController(&window);
    controller->setContext(window.context());

    const DemoScriptParser::Result parsed = DemoScriptParser::parse(
        QString::fromLatin1(kDemoScript));
    if (parsed.ok)
        controller->setSteps(DemoScriptParser::toControllerSteps(parsed.steps));

    // The DemoModeBar drives the controller and reflects its step indicator and
    // narration. It is docked at the bottom of the hub so it is always visible.
    DemoModeBar *bar = new DemoModeBar(&window);
    bar->setController(controller);

    QDockWidget *dock = new QDockWidget(QStringLiteral("Demo Mode"), &window);
    dock->setObjectName(QStringLiteral("demoModeDock"));
    dock->setWidget(bar);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    window.addDockWidget(Qt::BottomDockWidgetArea, dock);

    return app.exec();
}
