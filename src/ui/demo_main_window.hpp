// PatchOrchestrator — Sprint 1 (A1) unified demo main window.
//
// A QMainWindow that hosts the three existing UI widgets (dashboard, schedule
// editor, control panel) as tabs in a single application. The existing widget
// classes are reused as-is; their central widgets are embedded into a
// QTabWidget so all three panels render inside one window.

#ifndef PATCHORCHESTRATOR_UI_DEMO_MAIN_WINDOW_HPP
#define PATCHORCHESTRATOR_UI_DEMO_MAIN_WINDOW_HPP

#include <QMainWindow>

class QTabWidget;
class DashboardWindow;
class ScheduleEditorWindow;
class ControlPanelWindow;

class DemoMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DemoMainWindow(QWidget *parent = nullptr);

    // Accessors for the embedded panels (used by tests and wiring).
    DashboardWindow *dashboard() const { return m_dashboard; }
    ScheduleEditorWindow *scheduleEditor() const { return m_schedule; }
    ControlPanelWindow *controlPanel() const { return m_control; }
    QTabWidget *tabWidget() const { return m_tabs; }

private:
    void buildUi();

    QTabWidget *m_tabs;
    DashboardWindow *m_dashboard;
    ScheduleEditorWindow *m_schedule;
    ControlPanelWindow *m_control;
};

#endif // PATCHORCHESTRATOR_UI_DEMO_MAIN_WINDOW_HPP
