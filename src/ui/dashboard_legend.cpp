// PatchOrchestrator — Sprint 24 (C7) dashboard legend implementation.

#include "dashboard_legend.hpp"
#include "state_badge.hpp"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace {

// Entries wrap after this many columns instead of running in one long row,
// so the legend still reads cleanly at the dashboard's default (and
// typically laptop-constrained) window width instead of squeezing every
// badge below its own size hint.
constexpr int kColumns = 3;

} // namespace

DashboardLegend::DashboardLegend(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 6, 8, 6);
    root->setSpacing(6);

    m_titleLabel = new QLabel(QStringLiteral("Legend — State colors"), this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    root->addWidget(m_titleLabel);

    // One real StateBadge chip per known state, in the C2/C3 display order.
    // Because this is the same StateBadge widget the table's State column
    // renders — icon and label already baked into the one widget — the
    // legend can never drift out of visual sync with what it's explaining,
    // and there's no separate "= meaning" text repeating what the badge
    // already says. Wrapped into a grid (kColumns per row) rather than one
    // long row, so badges keep their full width regardless of the window's
    // width.
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(20);
    grid->setVerticalSpacing(8);

    m_states = StateBadge::legendStates();
    for (int i = 0; i < m_states.size(); ++i) {
        const QString &state = m_states.at(i);
        auto *badge = new StateBadge(state, this);
        grid->addWidget(badge, i / kColumns, i % kColumns, Qt::AlignLeft);
    }
    for (int c = 0; c < kColumns; ++c)
        grid->setColumnStretch(c, 1);
    root->addLayout(grid);
}

QString DashboardLegend::title() const
{
    return m_titleLabel != nullptr ? m_titleLabel->text() : QString();
}

int DashboardLegend::entryCount() const
{
    return m_states.size();
}

QString DashboardLegend::entryState(int index) const
{
    if (index < 0 || index >= m_states.size())
        return QString();
    return m_states.at(index);
}

QColor DashboardLegend::entryColor(int index) const
{
    if (index < 0 || index >= m_states.size())
        return QColor();  // invalid
    return StateBadge::colorForState(m_states.at(index));
}

QString DashboardLegend::entryLabel(int index) const
{
    if (index < 0 || index >= m_states.size())
        return QString();
    return StateBadge::labelForState(m_states.at(index));
}
