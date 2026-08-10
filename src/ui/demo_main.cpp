// PatchOrchestrator — Sprint 1 (A1) unified demo entry point.
//
// Constructs and shows the DemoMainWindow that hosts the dashboard, schedule
// editor, and control panel as tabs in a single application.

#include "demo_main_window.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("PatchOrchestrator"));
    QApplication::setApplicationName(QStringLiteral("PatchOrchestrator"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    DemoMainWindow window;
    window.show();

    return app.exec();
}
