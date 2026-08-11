// PatchOrchestrator — Sprint 27 (D3) seed config.
//
// A small, self-contained widget that lets the user set the deterministic seed
// used to reproduce a demo run. It hosts a QSpinBox and stores the chosen
// value in the shared DemoAppContext (Sprint 3 / A3), so the value is the
// single source of truth and other panels react to changes. The control reads
// the initial value from the context and stays in sync with it.

#ifndef PATCHORCHESTRATOR_UI_SEED_CONTROL_HPP
#define PATCHORCHESTRATOR_UI_SEED_CONTROL_HPP

#include <QWidget>

class QSpinBox;
class DemoAppContext;

class SeedControl : public QWidget
{
    Q_OBJECT

public:
    // context may be nullptr (standalone); call setContext() to bind it.
    explicit SeedControl(DemoAppContext *context = nullptr,
                         QWidget *parent = nullptr);

    // Bind (or rebind) this control to a shared app context. Passing nullptr
    // unbinds it. Reads the current seed from the context and keeps the spin
    // box and context in sync.
    void setContext(DemoAppContext *context);
    DemoAppContext *context() const { return m_context; }

    // Test accessors.
    QSpinBox *spinBox() const { return m_spinBox; }
    int seed() const;

private:
    QSpinBox *m_spinBox;
    DemoAppContext *m_context;
};

#endif // PATCHORCHESTRATOR_UI_SEED_CONTROL_HPP
