// PatchOrchestrator — Sprint 18 (C1) animated progress bar implementation.

#include "animated_progress_bar.hpp"

#include <QProgressBar>

namespace {

// Smoothstep easing: maps t in [0,1] to a smooth ease-in-out curve so the bar
// accelerates out of the start and decelerates into the target.
double smoothstep(double t)
{
    t = qBound(0.0, t, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

} // namespace

AnimatedProgressBar::AnimatedProgressBar(QWidget *parent)
    : QWidget(parent)
    , m_bar(new QProgressBar(this))
    , m_startValue(0)
    , m_targetValue(0)
    , m_durationMs(500)
{
    m_bar->setRange(0, 100);
    m_bar->setValue(0);
    m_bar->setTextVisible(false);

    connect(&m_timer, &QTimer::timeout, this, &AnimatedProgressBar::onTick);
}

int AnimatedProgressBar::value() const
{
    return m_bar->value();
}

void AnimatedProgressBar::setTarget(int target)
{
    target = qBound(0, target, 100);
    if (target == m_bar->value()) {
        // Already at the target; no animation needed.
        m_targetValue = target;
        m_timer.stop();
        return;
    }
    m_startValue = m_bar->value();
    m_targetValue = target;
    startAnimation();
}

void AnimatedProgressBar::startAnimation()
{
    m_clock.start();
    m_timer.start(16);
}

void AnimatedProgressBar::onTick()
{
    const qint64 elapsed = m_clock.elapsed();
    const double t = static_cast<double>(elapsed) / static_cast<double>(m_durationMs);
    if (t >= 1.0) {
        m_timer.stop();
        m_bar->setValue(m_targetValue);
        return;
    }
    const double eased = smoothstep(t);
    const int value = m_startValue + static_cast<int>(
        (m_targetValue - m_startValue) * eased);
    m_bar->setValue(value);
}
