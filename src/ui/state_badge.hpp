// PatchOrchestrator — Sprint 20 (C3) reusable state badge.
//
// A small, self-contained widget that renders a color-coded patch state as a
// rounded badge (colored pill with an icon and a human-readable label). It
// owns the state->color mapping (the C2 single source of truth) and also
// provides a legend explaining the color coding and state meanings, so it can
// be reused anywhere a state needs to be shown consistently.

#ifndef PATCHORCHESTRATOR_UI_STATE_BADGE_HPP
#define PATCHORCHESTRATOR_UI_STATE_BADGE_HPP

#include <QColor>
#include <QString>
#include <QStringList>
#include <QWidget>

class StateBadge : public QWidget
{
    Q_OBJECT

public:
    explicit StateBadge(const QString &state = QString(), QWidget *parent = nullptr);

    // Set the patch state this badge renders. Re-renders the badge.
    void setState(const QString &state);
    QString state() const { return m_state; }

    // Derived rendering data for the current state.
    QColor color() const { return colorForState(m_state); }
    QString label() const { return labelForState(m_state); }
    QString icon() const { return iconForState(m_state); }

    // Sprint 19 (C2) state->color mapping — single source of truth. Moved
    // here so the reusable badge owns the mapping; the dashboard delegates to
    // it so every row/badge stays consistent.
    static QColor colorForState(const QString &state);
    static QString labelForState(const QString &state);
    static QString iconForState(const QString &state);

    // Legend: the ordered list of known states and a human-readable legend
    // explaining the color coding and state meanings.
    static QStringList legendStates();
    static QString legendText();

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_state;
};

#endif // PATCHORCHESTRATOR_UI_STATE_BADGE_HPP
