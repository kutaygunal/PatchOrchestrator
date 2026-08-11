// PatchOrchestrator — Sprint 6 (A6) Demo Mode UI.
//
// A widget that drives a DemoModeController (S4) through a guided scripted
// walkthrough. It exposes Start / Next / Prev / Stop controls, a current-step
// indicator ("Step X of Y"), and a read-only narration text area. The controls
// call the corresponding controller methods, and the indicator + narration
// stay in sync with the controller via its stepChanged / narrationChanged /
// runningChanged signals.

#ifndef PATCHORCHESTRATOR_UI_DEMO_MODE_BAR_HPP
#define PATCHORCHESTRATOR_UI_DEMO_MODE_BAR_HPP

#include <QWidget>

class QLabel;
class QPushButton;
class QTextEdit;
class DemoModeController;

class DemoModeBar : public QWidget
{
    Q_OBJECT

public:
    explicit DemoModeBar(QWidget *parent = nullptr);

    // The controller this bar drives. Replacing it re-wires the controls and
    // refreshes the indicator + narration from the new controller.
    void setController(DemoModeController *controller);
    DemoModeController *controller() const { return m_controller; }

    // Accessors for tests and wiring.
    QPushButton *startButton() const { return m_start; }
    QPushButton *nextButton() const { return m_next; }
    QPushButton *prevButton() const { return m_prev; }
    QPushButton *stopButton() const { return m_stop; }
    QLabel *stepIndicator() const { return m_indicator; }
    QTextEdit *narrationArea() const { return m_narration; }

private slots:
    void onStart();
    void onNext();
    void onPrev();
    void onStop();
    void onStepChanged(int index, const QString &stepId);
    void onNarrationChanged(const QString &narration);
    void onRunningChanged(bool running);

private:
    void buildUi();
    void refreshIndicator();
    void refreshNarration();

    DemoModeController *m_controller;
    QPushButton *m_start;
    QPushButton *m_next;
    QPushButton *m_prev;
    QPushButton *m_stop;
    QLabel *m_indicator;
    QTextEdit *m_narration;
};

#endif // PATCHORCHESTRATOR_UI_DEMO_MODE_BAR_HPP
