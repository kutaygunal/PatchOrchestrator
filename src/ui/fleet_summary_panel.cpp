// PatchOrchestrator — Sprint 21 (C4) fleet summary panel implementation.

#include "fleet_summary_panel.hpp"
#include "state_badge.hpp"

#include <QGridLayout>
#include <QJsonObject>
#include <QLabel>

namespace {

// Default color used for the total row (neutral grey).
const QColor kTotalColor(0x42, 0x42, 0x42);

} // namespace

QStringList FleetSummaryPanel::states()
{
    return StateBadge::legendStates();
}

FleetSummaryPanel::FleetSummaryPanel(QWidget *parent)
    : QWidget(parent)
    , m_totalLabel(nullptr)
{
    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(2);

    // One row per known state: a colored label (from the C2/C3 mapping) and a
    // count that updates as the fleet changes.
    const QStringList stateList = states();
    for (int i = 0; i < stateList.size(); ++i) {
        const QString &state = stateList.at(i);

        auto *nameLabel = new QLabel(StateBadge::labelForState(state), this);
        nameLabel->setStyleSheet(
            QStringLiteral("color: %1; font-weight: bold;")
                .arg(StateBadge::colorForState(state).name()));
        layout->addWidget(nameLabel, 0, i * 2);

        auto *countLabel = new QLabel(QStringLiteral("0"), this);
        countLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        layout->addWidget(countLabel, 0, i * 2 + 1);
        m_countLabels.insert(state, countLabel);
    }

    // Total row spanning the full width.
    auto *totalName = new QLabel(QStringLiteral("Total"), this);
    totalName->setStyleSheet(
        QStringLiteral("color: %1; font-weight: bold;").arg(kTotalColor.name()));
    layout->addWidget(totalName, 1, 0);

    m_totalLabel = new QLabel(QStringLiteral("0"), this);
    m_totalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_totalLabel->setStyleSheet(
        QStringLiteral("color: %1; font-weight: bold;").arg(kTotalColor.name()));
    layout->addWidget(m_totalLabel, 1, 1);

    layout->setColumnStretch(layout->columnCount(), 1);
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
