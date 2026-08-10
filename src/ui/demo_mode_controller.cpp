// PatchOrchestrator — Sprint 4 (A4) Demo Mode engine.
//
// See demo_mode_controller.hpp for the contract. The controller is a pure
// QObject: it sequences a scripted step list, exposes the current step and its
// narration, and supports advance, pause, resume, stop, and auto-advance. It
// optionally mirrors the current step id into a shared DemoAppContext (A3).

#include "demo_mode_controller.hpp"
#include "demo_app_context.hpp"

#include <QTimer>

namespace {

const int kNoAutoAdvance = 0;

}  // namespace

DemoModeController::DemoModeController(QObject *parent)
    : QObject(parent)
    , m_index(0)
    , m_running(false)
    , m_paused(false)
    , m_complete(false)
    , m_autoAdvanceMs(kNoAutoAdvance)
    , m_timer(nullptr)
    , m_context(nullptr)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &DemoModeController::next);
}

void DemoModeController::setSteps(const QVector<DemoStep> &steps)
{
    stop();
    m_steps = steps;
}

QString DemoModeController::currentStepId() const
{
    if (m_index < 0 || m_index >= m_steps.size())
        return QString();
    return m_steps.at(m_index).id;
}

QString DemoModeController::narration() const
{
    if (m_index < 0 || m_index >= m_steps.size())
        return QString();
    return m_steps.at(m_index).narration;
}

void DemoModeController::setAutoAdvanceInterval(int ms)
{
    m_autoAdvanceMs = ms;
    if (m_running && !m_paused && m_autoAdvanceMs > 0)
        scheduleAutoAdvance();
}

void DemoModeController::setContext(DemoAppContext *context)
{
    m_context = context;
}

void DemoModeController::start()
{
    m_index = 0;
    m_running = true;
    m_paused = false;
    m_complete = false;
    applyStep();
    scheduleAutoAdvance();
}

void DemoModeController::next()
{
    if (!m_running || m_complete)
        return;

    if (m_index + 1 >= m_steps.size()) {
        // Advancing past the last step completes the walkthrough.
        m_complete = true;
        m_running = false;
        m_paused = false;
        stopAutoAdvance();
        emit completeChanged(true);
        emit runningChanged(false);
        emit pausedChanged(false);
        return;
    }

    ++m_index;
    applyStep();
    scheduleAutoAdvance();
}

void DemoModeController::prev()
{
    if (!m_running || m_complete || m_index <= 0)
        return;

    --m_index;
    applyStep();
    scheduleAutoAdvance();
}

void DemoModeController::pause()
{
    if (!m_running || m_paused)
        return;
    m_paused = true;
    stopAutoAdvance();
    emit pausedChanged(true);
}

void DemoModeController::resume()
{
    if (!m_running || !m_paused)
        return;
    m_paused = false;
    emit pausedChanged(false);
    scheduleAutoAdvance();
}

void DemoModeController::stop()
{
    const bool wasRunning = m_running;
    const bool wasPaused = m_paused;
    const bool wasComplete = m_complete;

    m_index = 0;
    m_running = false;
    m_paused = false;
    m_complete = false;
    stopAutoAdvance();

    if (wasRunning)
        emit runningChanged(false);
    if (wasPaused)
        emit pausedChanged(false);
    if (wasComplete)
        emit completeChanged(false);

    applyStep();
}

void DemoModeController::applyStep()
{
    emit stepChanged(m_index, currentStepId());
    emit narrationChanged(narration());

    // Mirror the current step into the shared app context (A3) so the rest of
    // the hub reflects the walkthrough position. The context's change-only
    // setters make echoing the same value a no-op.
    if (m_context)
        m_context->setRolloutState(currentStepId());
}

void DemoModeController::scheduleAutoAdvance()
{
    stopAutoAdvance();
    if (m_running && !m_paused && m_autoAdvanceMs > 0)
        m_timer->start(m_autoAdvanceMs);
}

void DemoModeController::stopAutoAdvance()
{
    if (m_timer->isActive())
        m_timer->stop();
}
