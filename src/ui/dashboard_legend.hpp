// PatchOrchestrator — Sprint 24 (C7) dashboard legend.
//
// A small, self-contained widget that explains the color coding and state
// meanings for demo viewers. It renders one entry per known patch state
// (succeeded/failed/paused/running/pending/rolled_back) using the C2/C3
// state->color mapping (owned by StateBadge), so the legend always stays in
// sync with the colors applied to rows/badges in the dashboard.

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
    QList<QLabel *> m_swatches;
    QList<QLabel *> m_labels;
    QLabel *m_titleLabel;
};

#endif // PATCHORCHESTRATOR_UI_DASHBOARD_LEGEND_HPP
