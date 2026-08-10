// PatchOrchestrator — Phase 10 control-actions UI entry point.
//
// Constructs and shows the ControlPanelWindow.

#include "control_panel.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("PatchOrchestrator"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    ControlPanelWindow window;
    window.show();

    return app.exec();
}
