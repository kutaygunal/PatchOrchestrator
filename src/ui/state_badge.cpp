// PatchOrchestrator — Sprint 20 (C3) reusable state badge implementation.

#include "state_badge.hpp"

#include <QFontMetrics>
#include <QPainter>

namespace {

// Default color for unknown/empty states (grey). Kept as a named constant so
// the mapping and the default branch stay in sync.
const QColor kDefaultStateColor(0x9e, 0x9e, 0x9e);

} // namespace

// Sprint 19 (C2) state->color mapping (single source of truth). Every badge
// and every dashboard row derives its color from this one function, so a
// state always maps to the same color everywhere.
QColor StateBadge::colorForState(const QString &state)
{
    if (state == QLatin1String("succeeded"))
        return QColor(0x2e, 0x7d, 0x32);  // green
    if (state == QLatin1String("failed"))
        return QColor(0xc6, 0x28, 0x28);  // red
    if (state == QLatin1String("paused"))
        return QColor(0xf9, 0xa8, 0x25);  // amber
    if (state == QLatin1String("running"))
        return QColor(0x15, 0x65, 0xc0);  // blue
    if (state == QLatin1String("pending"))
        return QColor(0x9e, 0x9e, 0x9e);  // grey
    if (state == QLatin1String("rolled_back"))
        return QColor(0x6a, 0x1b, 0x9a);  // purple
    // Unknown/empty states map to the defined default (grey) — no crash.
    return kDefaultStateColor;
}

QString StateBadge::labelForState(const QString &state)
{
    if (state == QLatin1String("succeeded"))
        return QStringLiteral("Succeeded");
    if (state == QLatin1String("failed"))
        return QStringLiteral("Failed");
    if (state == QLatin1String("paused"))
        return QStringLiteral("Paused");
    if (state == QLatin1String("running"))
        return QStringLiteral("Running");
    if (state == QLatin1String("pending"))
        return QStringLiteral("Pending");
    if (state == QLatin1String("rolled_back"))
        return QStringLiteral("Rolled back");
    return QStringLiteral("Unknown");
}

QString StateBadge::iconForState(const QString &state)
{
    if (state == QLatin1String("succeeded"))
        return QStringLiteral("\u2713");   // check mark
    if (state == QLatin1String("failed"))
        return QStringLiteral("\u2717");   // cross
    if (state == QLatin1String("paused"))
        return QStringLiteral("\u23F8");   // pause
    if (state == QLatin1String("running"))
        return QStringLiteral("\u25B6");   // play
    if (state == QLatin1String("pending"))
        return QStringLiteral("\u2022");   // bullet
    if (state == QLatin1String("rolled_back"))
        return QStringLiteral("\u21BA");   // counterclockwise arrow
    return QStringLiteral("?");
}

QStringList StateBadge::legendStates()
{
    return {QStringLiteral("succeeded"), QStringLiteral("failed"),
            QStringLiteral("paused"),    QStringLiteral("running"),
            QStringLiteral("pending"),   QStringLiteral("rolled_back")};
}

QString StateBadge::legendText()
{
    QString text;
    const QStringList states = legendStates();
    for (const QString &s : states) {
        text += QStringLiteral("%1 (%2) — %3\n")
                    .arg(labelForState(s), colorForState(s).name(), iconForState(s));
    }
    return text.trimmed();
}

StateBadge::StateBadge(const QString &state, QWidget *parent)
    : QWidget(parent)
    , m_state(state)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void StateBadge::setState(const QString &state)
{
    if (m_state == state)
        return;
    m_state = state;
    update();
}

QSize StateBadge::sizeHint() const
{
    QFontMetrics fm(font());
    const int textWidth = fm.horizontalAdvance(icon() + QLatin1Char(' ') + label());
    return QSize(textWidth + 24, fm.height() + 8);
}

void StateBadge::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRect r = rect().adjusted(2, 2, -2, -2);
    p.setPen(Qt::NoPen);
    p.setBrush(color());
    p.drawRoundedRect(r, r.height() / 2.0, r.height() / 2.0);

    QFont f = font();
    f.setBold(true);
    p.setFont(f);
    p.setPen(QColor(Qt::white));
    p.drawText(r, Qt::AlignCenter, icon() + QLatin1Char(' ') + label());
}
