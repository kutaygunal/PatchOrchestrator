// PatchOrchestrator — Phase 10 control-actions UI entry point.
//
// Constructs and shows the ControlPanelWindow.

#include "control_panel.hpp"
#include "theme.hpp"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("PatchOrchestrator"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    Theme::apply(app);
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/patchorchestrator_control.png")));

    ControlPanelWindow window;
    window.show();

    return app.exec();
}
