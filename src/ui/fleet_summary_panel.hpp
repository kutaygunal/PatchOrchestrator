// PatchOrchestrator — Sprint 21 (C4) fleet summary panel.
//
// A small summary widget that aggregates endpoint counts by patch state
// (succeeded/failed/paused/running/pending/rolled_back) and shows the total
// number of endpoints. It re-renders whenever the endpoint data changes, so
// the dashboard can keep the summary in sync with the live fleet.

#ifndef PATCHORCHESTRATOR_UI_FLEET_SUMMARY_PANEL_HPP
#define PATCHORCHESTRATOR_UI_FLEET_SUMMARY_PANEL_HPP

#include <QHash>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <QWidget>

class QLabel;

class FleetSummaryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FleetSummaryPanel(QWidget *parent = nullptr);

    // Recompute the per-state counts and the total from the given endpoint
    // list. Each endpoint is a JSON object carrying a "state" field. Updates
    // the displayed counts and total immediately.
    void setEndpoints(const QJsonArray &endpoints);

    // Test accessors.
    int countForState(const QString &state) const;
    int total() const;

    // The six known states (from the C2/C3 mapping), in display order.
    static QStringList states();

private:
    void rebuild();

    QHash<QString, int> m_counts;
    QHash<QString, QLabel *> m_countLabels;
    QLabel *m_totalLabel;
};

#endif // PATCHORCHESTRATOR_UI_FLEET_SUMMARY_PANEL_HPP
