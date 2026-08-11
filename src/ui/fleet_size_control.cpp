// PatchOrchestrator — Sprint 25 (D1) fleet size config implementation.

#include "fleet_size_control.hpp"
#include "demo_app_context.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

namespace {

// The spin box must always allow at least one endpoint (a fleet of zero makes
// no sense for a rollout). The upper bound is generous so large demo fleets
// are representable.
const int kMinFleetSize = 1;
const int kMaxFleetSize = 1000;
const int kDefaultFleetSize = 10;

}  // namespace

FleetSizeControl::FleetSizeControl(DemoAppContext *context, QWidget *parent)
    : QWidget(parent)
    , m_spinBox(nullptr)
    , m_context(nullptr)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(QStringLiteral("Fleet size (endpoints):"), this));

    m_spinBox = new QSpinBox(this);
    m_spinBox->setObjectName(QStringLiteral("fleetSizeSpinBox"));
    m_spinBox->setRange(kMinFleetSize, kMaxFleetSize);
    m_spinBox->setValue(kDefaultFleetSize);
    m_spinBox->setToolTip(
        QStringLiteral("Number of endpoints in the fleet before simulation."));
    layout->addWidget(m_spinBox);
    layout->addStretch(1);

    setContext(context);
}

void FleetSizeControl::setContext(DemoAppContext *context)
{
    // Disconnect any previous binding so rebinding does not double-fire.
    if (m_context != nullptr) {
        disconnect(m_context, nullptr, this, nullptr);
        disconnect(m_spinBox, nullptr, this, nullptr);
    }

    m_context = context;
    if (m_context == nullptr)
        return;

    // Read the initial value from the shared context.
    m_spinBox->setValue(m_context->fleetSize());

    // Propagate shared-state changes into the spin box.
    connect(m_context, &DemoAppContext::fleetSizeChanged, this,
            [this](int size) { m_spinBox->setValue(size); });

    // Write local edits back into the shared context (the change-only setter
    // makes the echo from fleetSizeChanged a no-op, so there is no loop).
    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int size) { m_context->setFleetSize(size); });
}

int FleetSizeControl::fleetSize() const
{
    return m_spinBox != nullptr ? m_spinBox->value() : 0;
}
