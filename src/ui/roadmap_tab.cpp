// PatchOrchestrator — Sprint 7 (A7) Roadmap/Future tab.
//
// See roadmap_tab.hpp for the contract. The tab renders each RoadmapItem as a
// styled card (title, status badge, target phase, description) inside a
// QScrollArea so the view scrolls when the model overflows the viewport. An
// empty model shows a placeholder label instead of crashing.

#include "roadmap_tab.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {

// Stylesheet applied to the roadmap view. Cards get a subtle border, rounded
// corners, and a colored status badge; the header row is bold and spaced.
const char kRoadmapStyle[] = R"(
    QScrollArea#roadmapScroll { background: #f4f6f8; border: none; }
    QWidget#roadmapContent { background: #f4f6f8; }
    QLabel#roadmapHeader {
        font-size: 18px; font-weight: bold; color: #1f2937;
        padding: 4px 0 8px 0;
    }
    QFrame#roadmapCard {
        background: #ffffff; border: 1px solid #d1d5db;
        border-radius: 8px; margin: 4px 0;
    }
    QLabel#cardTitle { font-size: 15px; font-weight: bold; color: #111827; }
    QLabel#cardStatus {
        font-size: 12px; font-weight: bold; color: #ffffff;
        background: #2563eb; border-radius: 10px; padding: 2px 10px;
    }
    QLabel#cardPhase { font-size: 12px; color: #6b7280; }
    QLabel#cardDescription { font-size: 13px; color: #374151; }
    QLabel#roadmapEmpty { font-size: 14px; color: #6b7280; padding: 24px; }
)";

}  // namespace

RoadmapTab::RoadmapTab(QWidget *parent)
    : QWidget(parent)
    , m_scroll(nullptr)
    , m_cardsLayout(nullptr)
    , m_empty(nullptr)
{
    buildUi();
}

void RoadmapTab::setItems(const QVector<RoadmapItem> &items)
{
    m_items = items;
    rebuild();
}

QString RoadmapTab::itemTitle(int index) const
{
    return (index >= 0 && index < m_items.size()) ? m_items.at(index).title
                                                  : QString();
}

QString RoadmapTab::itemDescription(int index) const
{
    return (index >= 0 && index < m_items.size()) ? m_items.at(index).description
                                                  : QString();
}

QString RoadmapTab::itemStatus(int index) const
{
    return (index >= 0 && index < m_items.size()) ? m_items.at(index).status
                                                  : QString();
}

QString RoadmapTab::itemTargetPhase(int index) const
{
    return (index >= 0 && index < m_items.size()) ? m_items.at(index).targetPhase
                                                  : QString();
}

void RoadmapTab::buildUi()
{
    setStyleSheet(QString::fromLatin1(kRoadmapStyle));

    // Header.
    QLabel *header = new QLabel(QStringLiteral("Roadmap — Future Vision"), this);
    header->setObjectName(QStringLiteral("roadmapHeader"));

    // Scrollable content area.
    m_scroll = new QScrollArea(this);
    m_scroll->setObjectName(QStringLiteral("roadmapScroll"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);

    QWidget *content = new QWidget;
    content->setObjectName(QStringLiteral("roadmapContent"));
    m_cardsLayout = new QVBoxLayout(content);
    m_cardsLayout->setContentsMargins(12, 8, 12, 12);
    m_cardsLayout->setSpacing(8);
    m_cardsLayout->addStretch();

    m_scroll->setWidget(content);

    // Empty-state placeholder (hidden until an empty model is set).
    m_empty = new QLabel(QStringLiteral("No roadmap items yet."), this);
    m_empty->setObjectName(QStringLiteral("roadmapEmpty"));
    m_empty->setAlignment(Qt::AlignCenter);
    m_empty->hide();

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(header);
    root->addWidget(m_scroll, /*stretch=*/1);
    root->addWidget(m_empty);
}

void RoadmapTab::rebuild()
{
    // Clear any previously rendered cards.
    while (QLayoutItem *item = m_cardsLayout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }

    if (m_items.isEmpty()) {
        m_empty->show();
        m_scroll->hide();
        return;
    }

    m_empty->hide();
    m_scroll->show();

    for (const RoadmapItem &item : m_items) {
        // Card frame.
        QFrame *card = new QFrame;
        card->setObjectName(QStringLiteral("roadmapCard"));

        // Title + status badge row.
        QLabel *title = new QLabel(item.title, card);
        title->setObjectName(QStringLiteral("cardTitle"));
        title->setWordWrap(true);

        QLabel *status = new QLabel(item.status, card);
        status->setObjectName(QStringLiteral("cardStatus"));

        QHBoxLayout *titleRow = new QHBoxLayout;
        titleRow->addWidget(title, /*stretch=*/1);
        titleRow->addWidget(status);

        // Target phase.
        QLabel *phase = new QLabel(
            QStringLiteral("Target phase: %1").arg(item.targetPhase), card);
        phase->setObjectName(QStringLiteral("cardPhase"));

        // Description.
        QLabel *description = new QLabel(item.description, card);
        description->setObjectName(QStringLiteral("cardDescription"));
        description->setWordWrap(true);

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(12, 10, 12, 10);
        cardLayout->setSpacing(6);
        cardLayout->addLayout(titleRow);
        cardLayout->addWidget(phase);
        cardLayout->addWidget(description);

        m_cardsLayout->insertWidget(m_cardsLayout->count() - 1, card);
    }
}
