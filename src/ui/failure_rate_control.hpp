// PatchOrchestrator — Sprint 26 (D2) failure rate config.
//
// A small, self-contained widget that lets the user set the per-endpoint
// failure rate (0.0–1.0) before simulation. It hosts a QSlider and a
// QDoubleSpinBox that stay in sync, and stores the chosen value in the shared
// DemoAppContext (Sprint 3 / A3), so the value is the single source of truth
// and other panels react to changes. The control reads the initial value from
// the context and stays in sync with it.

#ifndef PATCHORCHESTRATOR_UI_FAILURE_RATE_CONTROL_HPP
#define PATCHORCHESTRATOR_UI_FAILURE_RATE_CONTROL_HPP

#include <QDoubleSpinBox>
#include <QSlider>
#include <QWidget>

class DemoAppContext;

class FailureRateControl : public QWidget
{
    Q_OBJECT

public:
    // context may be nullptr (standalone); call setContext() to bind it.
    explicit FailureRateControl(DemoAppContext *context = nullptr,
                                QWidget *parent = nullptr);

    // Bind (or rebind) this control to a shared app context. Passing nullptr
    // unbinds it. Reads the current failure rate from the context and keeps
    // the slider, spin box, and context in sync.
    void setContext(DemoAppContext *context);
    DemoAppContext *context() const { return m_context; }

    // Test accessors.
    QSlider *slider() const { return m_slider; }
    QDoubleSpinBox *spinBox() const { return m_spinBox; }
    double failureRate() const;

private:
    QSlider *m_slider;
    QDoubleSpinBox *m_spinBox;
    DemoAppContext *m_context;
};

#endif // PATCHORCHESTRATOR_UI_FAILURE_RATE_CONTROL_HPP
