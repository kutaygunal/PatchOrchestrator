// PatchOrchestrator — Phase 9 schedule-definition UI entry point.
//
// Constructs and shows the ScheduleEditorWindow.

#include "schedule_editor.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("PatchOrchestrator"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    ScheduleEditorWindow window;
    window.show();

    return app.exec();
}
