// PatchOrchestrator — Sprint 29 (D5) scenario selector implementation.

#include "scenario_selector.hpp"
#include "demo_app_context.hpp"
#include "demo_scenario.hpp"
#include "scenario_presets.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>

namespace {

// The placeholder first entry guides the user; it is not a real preset and is
// ignored by the selection handler.
//
// This must be a QStringLiteral (UTF-16 at compile time), not a `const
// char[]` handed to QLatin1String: MSVC encodes the \u2026 escape in a narrow
// string literal as UTF-8 bytes, and QLatin1String then reinterprets those
// bytes one-per-character, rendering the ellipsis as "\u00e2\u20ac\u00a6" in the UI.
QString placeholderText()
{
    return QStringLiteral("Select scenario\u2026");
}

}  // namespace

ScenarioSelector::ScenarioSelector(DemoAppContext *context, QWidget *parent)
    : QWidget(parent)
    , m_comboBox(nullptr)
    , m_context(nullptr)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(QStringLiteral("Scenario:"), this));

    m_comboBox = new QComboBox(this);
    m_comboBox->setObjectName(QStringLiteral("scenarioComboBox"));
    m_comboBox->setToolTip(
        QStringLiteral("Load a preset scenario into the config controls."));
    layout->addWidget(m_comboBox, 1);

    rebuildPresetList();

    setContext(context);
}

void ScenarioSelector::rebuildPresetList()
{
    // Remember the currently selected name (if any) so we can restore the
    // selection after rebuilding, then repopulate from the D4 presets.
    const QString selected = m_comboBox->currentText();

    m_comboBox->clear();
    m_comboBox->addItem(placeholderText());
    for (const DemoScenario &preset : scenario_presets::all()) {
        m_comboBox->addItem(preset.name);
    }

    // Restore the previous selection if it is still a listed preset; otherwise
    // leave the placeholder selected.
    const int index = m_comboBox->findText(selected);
    m_comboBox->setCurrentIndex(index >= 0 ? index : 0);
}

void ScenarioSelector::setContext(DemoAppContext *context)
{
    // Disconnect any previous binding so rebinding does not double-fire.
    if (m_context != nullptr)
        disconnect(m_comboBox, nullptr, this, nullptr);

    m_context = context;

    // Rebuild the preset list whenever the widget (re)binds, so the entries
    // always reflect the current presets. This is safe: the combo box's
    // selection handler only applies real presets.
    rebuildPresetList();

    if (m_context == nullptr)
        return;

    // When the user selects a preset, apply its fleet size, failure rate, and
    // seed to the shared context. This propagates to the bound D1/D2/D3
    // controls, overriding any manually set values. The placeholder entry is
    // ignored.
    connect(m_comboBox, &QComboBox::currentIndexChanged, this,
            [this](int index) {
                if (m_context == nullptr || index <= 0)
                    return;
                const QString name = m_comboBox->itemText(index);
                const DemoScenario preset = scenario_presets::byName(name);
                if (preset.name.isEmpty())
                    return;
                m_context->applyScenario(preset);
            });
}

int ScenarioSelector::presetCount() const
{
    // The list contains one placeholder plus one entry per preset. The preset
    // count is therefore the number of entries minus the placeholder.
    return scenario_presets::all().size();
}
