// PatchOrchestrator — Sprint 21 (C4) fleet summary panel implementation.
//
// Renders as a row of KPI tiles (one per state, plus a Total tile) rather
// than a flat line of "Label  Count" pairs: each tile carries a colored top
// accent matching the state's StateBadge color, a small muted state name,
// and a large count — the same "stat row" shape as the commercial
// monitoring dashboards this tool is modeled on.

#include "fleet_summary_panel.hpp"
#include "state_badge.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QVBoxLayout>

namespace {

// Build one KPI tile: a colored top accent bar, a small muted state name,
// and a large count label. Returns the tile frame; countLabel receives the
// (as-yet-unset) count QLabel so the caller can wire it into m_countLabels.
QFrame *makeTile(const QString &name, const QColor &accent, QLabel *&countLabel,
                  QWidget *parent)
{
    auto *tile = new QFrame(parent);
    tile->setObjectName(QStringLiteral("summaryTile"));
    tile->setStyleSheet(QStringLiteral(
        "QFrame#summaryTile {"
        "  background: #171c25;"
        "  border: 1px solid #2a3140;"
        "  border-top: 3px solid %1;"
        "  border-radius: 8px;"
        "}")
            .arg(accent.name()));

    auto *layout = new QVBoxLayout(tile);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(2);

    auto *nameLabel = new QLabel(name.toUpper(), tile);
    nameLabel->setStyleSheet(QStringLiteral("color: #97a1b3; font-size: 11px; font-weight: 600;"));
    layout->addWidget(nameLabel);

    countLabel = new QLabel(QStringLiteral("0"), tile);
    countLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 20px; font-weight: 700;")
                                   .arg(accent.name()));
    layout->addWidget(countLabel);

    return tile;
}

} // namespace

QStringList FleetSummaryPanel::states()
{
    return StateBadge::legendStates();
}

FleetSummaryPanel::FleetSummaryPanel(QWidget *parent)
    : QWidget(parent)
    , m_totalLabel(nullptr)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    // One tile per known state, colored with the C2/C3 mapping so it always
    // matches the badges in the table below.
    const QStringList stateList = states();
    for (const QString &state : stateList) {
        QLabel *countLabel = nullptr;
        QFrame *tile = makeTile(StateBadge::labelForState(state),
                                 StateBadge::colorForState(state), countLabel, this);
        layout->addWidget(tile, 1);
        m_countLabels.insert(state, countLabel);
    }

    // Total tile: neutral accent, visually the "sum" anchor at the end of the
    // row rather than just another state.
    QFrame *totalTile = makeTile(QStringLiteral("Total"), QColor(0x97, 0xa1, 0xb3),
                                  m_totalLabel, this);
    layout->addWidget(totalTile, 1);
}

void FleetSummaryPanel::setEndpoints(const QJsonArray &endpoints)
{
    m_counts.clear();
    for (const auto &value : endpoints) {
        const QJsonObject ep = value.toObject();
        const QString state = ep.value(QStringLiteral("state")).toString();
        m_counts[state] = m_counts.value(state) + 1;
    }
    rebuild();
}

int FleetSummaryPanel::countForState(const QString &state) const
{
    return m_counts.value(state, 0);
}

int FleetSummaryPanel::total() const
{
    int sum = 0;
    for (int count : m_counts)
        sum += count;
    return sum;
}

void FleetSummaryPanel::rebuild()
{
    const QStringList stateList = states();
    for (const QString &state : stateList) {
        QLabel *label = m_countLabels.value(state, nullptr);
        if (label != nullptr)
            label->setText(QString::number(countForState(state)));
    }
    if (m_totalLabel != nullptr)
        m_totalLabel->setText(QString::number(total()));
}
