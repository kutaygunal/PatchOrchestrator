// PatchOrchestrator — Sprint 1 (A1) unified demo main window.
//
// Embeds the three existing UI widgets (dashboard, schedule editor, control
// panel) as tabs in a single QTabWidget. The existing widget classes are
// reused unchanged; only their central widgets are reparented into the tab
// container so all three panels render inside one window.

#include "demo_main_window.hpp"
#include "control_panel.hpp"
#include "dashboard.hpp"
#include "schedule_editor.hpp"

#include <QTabWidget>

DemoMainWindow::DemoMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabs(nullptr)
    , m_dashboard(nullptr)
    , m_schedule(nullptr)
    , m_control(nullptr)
{
    setWindowTitle(QStringLiteral("PatchOrchestrator — Demo Hub"));
    resize(900, 640);

    buildUi();
}

void DemoMainWindow::buildUi()
{
    m_tabs = new QTabWidget(this);
    m_tabs->setDocumentMode(true);
    setCentralWidget(m_tabs);

    // Construct each existing panel as a child of this window so the embedded
    // widgets remain owned by the demo window, then embed their central
    // widgets into the tab container.
    m_dashboard = new DashboardWindow(this);
    m_schedule = new ScheduleEditorWindow(this);
    m_control = new ControlPanelWindow(this);

    m_tabs->addTab(m_dashboard->centralWidget(), QStringLiteral("Dashboard"));
    m_tabs->addTab(m_schedule->centralWidget(), QStringLiteral("Schedule Editor"));
    m_tabs->addTab(m_control->centralWidget(), QStringLiteral("Control Panel"));
}
