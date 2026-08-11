// PatchOrchestrator — Sprint 24 (C7) dashboard legend implementation.

#include "dashboard_legend.hpp"
#include "state_badge.hpp"

#include <QGridLayout>
#include <QLabel>

DashboardLegend::DashboardLegend(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
{
    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(2);

    m_titleLabel = new QLabel(QStringLiteral("Legend — State colors"), this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));
    layout->addWidget(m_titleLabel, 0, 0, 1, 2);

    // One entry per known state, in the C2/C3 display order. Each entry shows
    // a color swatch (from the single source of truth) and a human-readable
    // label/meaning.
    m_states = StateBadge::legendStates();
    for (int i = 0; i < m_states.size(); ++i) {
        const QString &state = m_states.at(i);

        auto *swatch = new QLabel(this);
        swatch->setFixedSize(16, 16);
        swatch->setStyleSheet(
            QStringLiteral("background-color: %1; border: 1px solid #888;")
                .arg(StateBadge::colorForState(state).name()));
        layout->addWidget(swatch, i + 1, 0);
        m_swatches.append(swatch);

        auto *label = new QLabel(
            QStringLiteral("%1 — %2")
                .arg(StateBadge::labelForState(state), StateBadge::iconForState(state)),
            this);
        layout->addWidget(label, i + 1, 1);
        m_labels.append(label);
    }

    layout->setColumnStretch(1, 1);
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
