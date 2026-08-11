// PatchOrchestrator — Sprint 7 (A7) Roadmap/Future tab.
//
// A widget that presents the project's future vision (persistence, auth, real
// fleet integration, observability, multi-tenant) as a styled, scrollable
// view. It consumes a minimal data-model contract (RoadmapItem) so the A8
// content model can plug in later without changing this widget.

#ifndef PATCHORCHESTRATOR_UI_ROADMAP_TAB_HPP
#define PATCHORCHESTRATOR_UI_ROADMAP_TAB_HPP

#include <QString>
#include <QVector>
#include <QWidget>

class QLabel;
class QScrollArea;
class QVBoxLayout;

// A single roadmap entry. This is the minimal contract the tab consumes; the
// A8 content model will populate these from JSON/C++ structs.
struct RoadmapItem
{
    QString title;
    QString description;
    QString status;        // e.g. "Planned", "In progress", "Backlog".
    QString targetPhase;   // e.g. "Phase B", "Phase C".
};

class RoadmapTab : public QWidget
{
    Q_OBJECT

public:
    explicit RoadmapTab(QWidget *parent = nullptr);

    // The roadmap model. Replacing it rebuilds the rendered entries.
    void setItems(const QVector<RoadmapItem> &items);
    QVector<RoadmapItem> items() const { return m_items; }

    // Accessors for tests and wiring.
    QScrollArea *scrollArea() const { return m_scroll; }
    int itemCount() const { return m_items.size(); }
    QString itemTitle(int index) const;
    QString itemDescription(int index) const;
    QString itemStatus(int index) const;
    QString itemTargetPhase(int index) const;
    QLabel *emptyLabel() const { return m_empty; }

private:
    void buildUi();
    void rebuild();

    QVector<RoadmapItem> m_items;
    QScrollArea *m_scroll;
    QVBoxLayout *m_cardsLayout;
    QLabel *m_empty;
};

#endif // PATCHORCHESTRATOR_UI_ROADMAP_TAB_HPP
