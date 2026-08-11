// PatchOrchestrator — Sprint 6 (A6) Demo Mode UI.
//
// See demo_mode_bar.hpp for the contract. The bar is a thin view over a
// DemoModeController: its buttons invoke the controller's start/next/prev/stop
// slots, and it reflects the controller's current step and narration through
// the step indicator label and the read-only narration text area.

#include "demo_mode_bar.hpp"
#include "demo_mode_controller.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

const char kStoppedText[] = "Stopped";

}  // namespace

DemoModeBar::DemoModeBar(QWidget *parent)
    : QWidget(parent)
    , m_controller(nullptr)
    , m_start(nullptr)
    , m_next(nullptr)
    , m_prev(nullptr)
    , m_stop(nullptr)
    , m_indicator(nullptr)
    , m_narration(nullptr)
{
    buildUi();
}

void DemoModeBar::setController(DemoModeController *controller)
{
    if (m_controller == controller)
        return;

    if (m_controller) {
        disconnect(m_controller, nullptr, this, nullptr);
    }

    m_controller = controller;

    if (m_controller) {
        connect(m_controller, &DemoModeController::stepChanged,
                this, &DemoModeBar::onStepChanged);
        connect(m_controller, &DemoModeController::narrationChanged,
                this, &DemoModeBar::onNarrationChanged);
        connect(m_controller, &DemoModeController::runningChanged,
                this, &DemoModeBar::onRunningChanged);
    }

    refreshIndicator();
    refreshNarration();
}

void DemoModeBar::buildUi()
{
    // Control row: Start / Prev / Next / Stop.
    m_start = new QPushButton(QStringLiteral("Start"), this);
    m_next = new QPushButton(QStringLiteral("Next"), this);
    m_prev = new QPushButton(QStringLiteral("Prev"), this);
    m_stop = new QPushButton(QStringLiteral("Stop"), this);

    connect(m_start, &QPushButton::clicked, this, &DemoModeBar::onStart);
    connect(m_next, &QPushButton::clicked, this, &DemoModeBar::onNext);
    connect(m_prev, &QPushButton::clicked, this, &DemoModeBar::onPrev);
    connect(m_stop, &QPushButton::clicked, this, &DemoModeBar::onStop);

    // Current-step indicator.
    m_indicator = new QLabel(QString::fromLatin1(kStoppedText), this);
    m_indicator->setObjectName(QStringLiteral("stepIndicator"));

    QHBoxLayout *controls = new QHBoxLayout;
    controls->addWidget(m_start);
    controls->addWidget(m_prev);
    controls->addWidget(m_next);
    controls->addWidget(m_stop);
    controls->addStretch();
    controls->addWidget(m_indicator);

    // Read-only narration text area.
    m_narration = new QTextEdit(this);
    m_narration->setObjectName(QStringLiteral("narrationArea"));
    m_narration->setReadOnly(true);
    m_narration->setPlaceholderText(QStringLiteral("Narration will appear here."));

    QVBoxLayout *root = new QVBoxLayout(this);
    root->addLayout(controls);
    root->addWidget(m_narration, /*stretch=*/1);
}

void DemoModeBar::onStart()
{
    if (m_controller)
        m_controller->start();
}

void DemoModeBar::onNext()
{
    if (m_controller)
        m_controller->next();
}

void DemoModeBar::onPrev()
{
    if (m_controller)
        m_controller->prev();
}

void DemoModeBar::onStop()
{
    if (m_controller)
        m_controller->stop();
}

void DemoModeBar::onStepChanged(int index, const QString &stepId)
{
    Q_UNUSED(index);
    Q_UNUSED(stepId);
    refreshIndicator();
}

void DemoModeBar::onNarrationChanged(const QString &narration)
{
    Q_UNUSED(narration);
    refreshNarration();
}

void DemoModeBar::onRunningChanged(bool running)
{
    Q_UNUSED(running);
    refreshIndicator();
}

void DemoModeBar::refreshIndicator()
{
    if (!m_controller) {
        m_indicator->setText(QString::fromLatin1(kStoppedText));
        return;
    }

    if (!m_controller->isRunning()) {
        m_indicator->setText(QString::fromLatin1(kStoppedText));
        return;
    }

    const int total = m_controller->steps().size();
    const int current = m_controller->currentIndex() + 1;
    m_indicator->setText(QStringLiteral("Step %1 of %2").arg(current).arg(total));
}

void DemoModeBar::refreshNarration()
{
    if (!m_controller) {
        m_narration->clear();
        return;
    }
    m_narration->setPlainText(m_controller->narration());
}
