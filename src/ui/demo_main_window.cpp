// PatchOrchestrator — Sprint 1/2 (A1/A2) unified demo main window.
//
// Embeds the three existing UI widgets (dashboard, schedule editor, control
// panel) as tabs in a single QTabWidget. The existing widget classes are
// reused unchanged; the panel windows themselves are embedded as tab pages so
// all three panels render inside one window.
//
// Sprint 2 (A2) adds layout persistence: the window state (QMainWindow::
// saveState) and the tab order are written to QSettings and restored on the
// next construction, so the layout survives restarts and the tabs remain
// reorderable.

#include "demo_main_window.hpp"
#include "control_panel.hpp"
#include "dashboard.hpp"
#include "schedule_editor.hpp"

#include <QSettings>
#include <QStringList>
#include <QTabBar>
#include <QTabWidget>

namespace {

// QSettings keys under the "layout" group.
const char kLayoutGroup[] = "layout";
const char kStateKey[] = "state";
const char kTabOrderKey[] = "tabOrder";

}  // namespace

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
    restoreLayout();
}

void DemoMainWindow::buildUi()
{
    m_tabs = new QTabWidget(this);
    m_tabs->setDocumentMode(true);
    m_tabs->setMovable(true);
    setCentralWidget(m_tabs);

    // Construct each existing panel as a child of this window, then embed the
    // panel windows themselves as tab pages. Embedding the windows (rather
    // than reparenting their central widgets) keeps each panel's QMainWindow
    // the sole owner of its central widget, so there is no double-delete and
    // no dangling central-widget pointer when the tab container is destroyed.
    m_dashboard = new DashboardWindow(this);
    m_schedule = new ScheduleEditorWindow(this);
    m_control = new ControlPanelWindow(this);

    m_tabs->addTab(m_dashboard, QStringLiteral("Dashboard"));
    m_tabs->addTab(m_schedule, QStringLiteral("Schedule Editor"));
    m_tabs->addTab(m_control, QStringLiteral("Control Panel"));
}

QByteArray DemoMainWindow::saveLayout() const
{
    const QByteArray state = saveState();

    QSettings settings;
    settings.beginGroup(QLatin1String(kLayoutGroup));
    settings.setValue(QLatin1String(kStateKey), state);

    QStringList order;
    for (int i = 0; i < m_tabs->count(); ++i)
        order << m_tabs->tabText(i);
    settings.setValue(QLatin1String(kTabOrderKey), order);
    settings.endGroup();
    settings.sync();

    return state;
}

void DemoMainWindow::restoreLayout()
{
    QSettings settings;
    settings.beginGroup(QLatin1String(kLayoutGroup));
    const QByteArray state = settings.value(QLatin1String(kStateKey)).toByteArray();
    const QStringList order = settings.value(QLatin1String(kTabOrderKey)).toStringList();
    settings.endGroup();

    if (!state.isEmpty())
        restoreState(state);
    restoreTabOrder(order);
}

void DemoMainWindow::restoreTabOrder(const QStringList &order)
{
    if (order.isEmpty())
        return;

    // Stably reorder the tabs so each position i holds the tab whose title
    // matches the saved order at i. moveTab shifts the intervening tabs right,
    // preserving the relative order of the remaining tabs.
    for (int i = 0; i < order.size() && i < m_tabs->count(); ++i) {
        const QString title = order.at(i);
        for (int j = i; j < m_tabs->count(); ++j) {
            if (m_tabs->tabText(j) == title) {
                if (j != i)
                    m_tabs->tabBar()->moveTab(j, i);
                break;
            }
        }
    }
}
