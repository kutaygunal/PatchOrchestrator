// PatchOrchestrator — Sprint 4 (A4) Demo Mode engine.
//
// A QObject that runs a guided scripted walkthrough: it holds an ordered list
// of steps (each with an id and narration text), exposes the current step and
// its narration, and supports advance, pause, resume, stop, and auto-advance.
//
// The core sequencing/narration logic is self-contained and testable without
// any UI. It may optionally drive the shared DemoAppContext (A3) so the rest
// of the hub reflects the current walkthrough step.

#ifndef PATCHORCHESTRATOR_UI_DEMO_MODE_CONTROLLER_HPP
#define PATCHORCHESTRATOR_UI_DEMO_MODE_CONTROLLER_HPP

#include <QObject>
#include <QString>
#include <QVector>

class QTimer;
class DemoAppContext;

// A single scripted walkthrough step: a stable id plus the narration text
// shown to the viewer while the controller is on this step.
struct DemoStep
{
    QString id;
    QString narration;
};

class DemoModeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY stepChanged)
    Q_PROPERTY(QString currentStepId READ currentStepId NOTIFY stepChanged)
    Q_PROPERTY(QString narration READ narration NOTIFY narrationChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)
    Q_PROPERTY(bool complete READ isComplete NOTIFY completeChanged)

public:
    explicit DemoModeController(QObject *parent = nullptr);

    // The ordered scripted step list. Replacing it resets the controller to
    // its initial (stopped) state.
    void setSteps(const QVector<DemoStep> &steps);
    QVector<DemoStep> steps() const { return m_steps; }

    int currentIndex() const { return m_index; }
    QString currentStepId() const;
    QString narration() const;
    bool isRunning() const { return m_running; }
    bool isPaused() const { return m_paused; }
    bool isComplete() const { return m_complete; }

    // Auto-advance delay in milliseconds. A value <= 0 disables auto-advance
    // (steps only move on explicit next()/prev() calls).
    void setAutoAdvanceInterval(int ms);
    int autoAdvanceInterval() const { return m_autoAdvanceMs; }

    // Optional shared app context (A3) driven during the walkthrough. When
    // set, the current step id is mirrored into the context's rollout state.
    void setContext(DemoAppContext *context);
    DemoAppContext *context() const { return m_context; }

public slots:
    // Begin the walkthrough from the first step.
    void start();
    // Advance to the next step; advancing past the last step completes it.
    void next();
    // Return to the previous step (no-op at the first step).
    void prev();
    // Halt auto-advance; the current step does not change while paused.
    void pause();
    // Resume auto-advance from the paused step.
    void resume();
    // Reset to a defined stopped state (back to the start, not running).
    void stop();

signals:
    void stepChanged(int index, const QString &stepId);
    void narrationChanged(const QString &narration);
    void runningChanged(bool running);
    void pausedChanged(bool paused);
    void completeChanged(bool complete);

private:
    void applyStep();
    void scheduleAutoAdvance();
    void stopAutoAdvance();

    QVector<DemoStep> m_steps;
    int m_index;
    bool m_running;
    bool m_paused;
    bool m_complete;
    int m_autoAdvanceMs;
    QTimer *m_timer;
    DemoAppContext *m_context;
};

#endif // PATCHORCHESTRATOR_UI_DEMO_MODE_CONTROLLER_HPP
