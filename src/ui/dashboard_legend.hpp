// PatchOrchestrator — Sprint 24 (C7) dashboard legend.
//
// A small, self-contained widget that explains the color coding and state
// meanings for demo viewers. It renders one entry per known patch state
// (succeeded/failed/paused/running/pending/rolled_back) as an actual
// StateBadge chip — the exact widget the dashboard's table renders into its
// State column — labeled with its meaning, so the legend doesn't just
// describe the badges, it *is* one of each of them.

#ifndef PATCHORCHESTRATOR_UI_DASHBOARD_LEGEND_HPP
#define PATCHORCHESTRATOR_UI_DASHBOARD_LEGEND_HPP

#include <QColor>
#include <QList>
#include <QString>
#include <QStringList>
#include <QWidget>

class QLabel;

class DashboardLegend : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardLegend(QWidget *parent = nullptr);

    // The legend title shown at the top of the widget.
    QString title() const;

    // Test accessors: the number of state entries and per-entry data.
    int entryCount() const;
    QString entryState(int index) const;
    QColor entryColor(int index) const;
    QString entryLabel(int index) const;

private:
    QStringList m_states;
    QLabel *m_titleLabel;
};

#endif // PATCHORCHESTRATOR_UI_DASHBOARD_LEGEND_HPP
