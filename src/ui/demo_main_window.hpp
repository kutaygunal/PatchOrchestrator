// PatchOrchestrator — Sprint 1/2 (A1/A2) unified demo main window.
//
// A QMainWindow that hosts the three existing UI widgets (dashboard, schedule
// editor, control panel) as tabs in a single application. The existing widget
// classes are reused as-is; the panel windows themselves are embedded as tab
// pages so all three panels render inside one window.
//
// Sprint 2 (A2) adds layout-state persistence: the window geometry/state and
// the tab order are saved to QSettings and restored on the next launch, so
// the layout survives restarts and the tabs remain reorderable.

#ifndef PATCHORCHESTRATOR_UI_DEMO_MAIN_WINDOW_HPP
#define PATCHORCHESTRATOR_UI_DEMO_MAIN_WINDOW_HPP

#include <QByteArray>
#include <QMainWindow>

class QTabWidget;
class AuditLogPanel;
class DashboardWindow;
class ScheduleEditorWindow;
class ControlPanelWindow;
class RoadmapTab;
class DemoAppContext;

class DemoMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DemoMainWindow(QWidget *parent = nullptr);

    // Accessors for the embedded panels (used by tests and wiring).
    DashboardWindow *dashboard() const { return m_dashboard; }
    ScheduleEditorWindow *scheduleEditor() const { return m_schedule; }
    ControlPanelWindow *controlPanel() const { return m_control; }
    RoadmapTab *roadmap() const { return m_roadmap; }
    AuditLogPanel *auditLog() const { return m_auditLog; }
    QTabWidget *tabWidget() const { return m_tabs; }

    // Sprint 3 (A3): the single shared app context bound to all three panels.
    DemoAppContext *context() const { return m_context; }

    // Layout persistence (Sprint 2 / A2). saveLayout() writes the current
    // window state and tab order to QSettings and returns the serialized
    // state; restoreLayout() reads them back and reapplies them.
    QByteArray saveLayout() const;
    void restoreLayout();

private:
    void buildUi();
    void restoreTabOrder(const QStringList &order);

    QTabWidget *m_tabs;
    DashboardWindow *m_dashboard;
    ScheduleEditorWindow *m_schedule;
    ControlPanelWindow *m_control;
    RoadmapTab *m_roadmap;
    AuditLogPanel *m_auditLog;
    DemoAppContext *m_context;
};

#endif // PATCHORCHESTRATOR_UI_DEMO_MAIN_WINDOW_HPP
