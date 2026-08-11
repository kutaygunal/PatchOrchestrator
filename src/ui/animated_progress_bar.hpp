// PatchOrchestrator — Sprint 18 (C1) animated progress bar.
//
// A QProgressBar that smoothly animates to a target value instead of jumping
// instantly. Each instance owns its own animation timer, so multiple bars
// animate independently (per-endpoint). setTarget() starts an interpolation
// from the current value to the new target; if a previous animation is still
// in flight it is retargeted from the current value.

#ifndef PATCHORCHESTRATOR_UI_ANIMATED_PROGRESS_BAR_HPP
#define PATCHORCHESTRATOR_UI_ANIMATED_PROGRESS_BAR_HPP

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>

class QProgressBar;

class AnimatedProgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit AnimatedProgressBar(QWidget *parent = nullptr);

    // Animate to the given target (clamped to 0..100). If already at the
    // target, no animation runs. If a previous animation is in flight, it is
    // retargeted from the current value.
    void setTarget(int target);
    int target() const { return m_targetValue; }

    // Current animated value (0..100).
    int value() const;

    bool isAnimating() const { return m_timer.isActive(); }

    // Animation duration in milliseconds (default 500).
    void setDurationMs(int ms) { m_durationMs = qMax(1, ms); }
    int durationMs() const { return m_durationMs; }

    // The underlying QProgressBar (for styling / direct access).
    QProgressBar *bar() const { return m_bar; }

private slots:
    void onTick();

private:
    void startAnimation();

    QProgressBar *m_bar;
    QTimer m_timer;
    QElapsedTimer m_clock;
    int m_startValue;
    int m_targetValue;
    int m_durationMs;
};

#endif // PATCHORCHESTRATOR_UI_ANIMATED_PROGRESS_BAR_HPP
