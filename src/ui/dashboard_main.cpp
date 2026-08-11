// PatchOrchestrator — Phase 8 Qt dashboard entry point.
//
// Constructs and shows the read-only DashboardWindow.

#include "dashboard.hpp"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("PatchOrchestrator"));
    QApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/patchorchestrator_dashboard.png")));

    DashboardWindow window;
    window.show();

    return app.exec();
}
