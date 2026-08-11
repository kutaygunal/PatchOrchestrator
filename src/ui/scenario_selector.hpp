// PatchOrchestrator — Sprint 29 (D5) scenario selector.
//
// A small, self-contained widget that lets the user load a predefined demo
// scenario (Sprint 28 / D4) into the shared config controls. It hosts a
// QComboBox listing every preset from scenario_presets::all() (plus a
// placeholder entry) and, when the user selects a preset, applies its fleet
// size, failure rate, and seed to the shared DemoAppContext (Sprint 3 / A3)
// via DemoAppContext::applyScenario. Because the D1/D2/D3 controls are bound
// to the same shared context, they update to match the preset, overriding any
// manually set values.

#ifndef PATCHORCHESTRATOR_UI_SCENARIO_SELECTOR_HPP
#define PATCHORCHESTRATOR_UI_SCENARIO_SELECTOR_HPP

#include <QWidget>

class QComboBox;
class DemoAppContext;

class ScenarioSelector : public QWidget
{
    Q_OBJECT

public:
    // context may be nullptr (standalone); call setContext() to bind it.
    explicit ScenarioSelector(DemoAppContext *context = nullptr,
                              QWidget *parent = nullptr);

    // Bind (or rebind) this selector to a shared app context. Passing nullptr
    // unbinds it. Selecting a preset then applies it to the context.
    void setContext(DemoAppContext *context);
    DemoAppContext *context() const { return m_context; }

    // Test accessors.
    QComboBox *comboBox() const { return m_comboBox; }

    // Number of predefined scenario presets listed in the combo box (i.e. the
    // count from scenario_presets::all(), not counting the placeholder entry).
    int presetCount() const;

private:
    void rebuildPresetList();

    QComboBox *m_comboBox;
    DemoAppContext *m_context;
};

#endif // PATCHORCHESTRATOR_UI_SCENARIO_SELECTOR_HPP
