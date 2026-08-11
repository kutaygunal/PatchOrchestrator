// PatchOrchestrator — Sprint 27 (D3) seed config implementation.

#include "seed_control.hpp"
#include "demo_app_context.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

namespace {

// The seed is a non-negative integer used to drive the deterministic engine so
// a demo is reproducible. The range is deliberately generous; 0 is a valid,
// documented default that gives a fresh (but still deterministic) run.
const int kMinSeed = 0;
const int kMaxSeed = 99999;
const int kDefaultSeed = 0;

}  // namespace

SeedControl::SeedControl(DemoAppContext *context, QWidget *parent)
    : QWidget(parent)
    , m_spinBox(nullptr)
    , m_context(nullptr)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(QStringLiteral("Seed:"), this));

    m_spinBox = new QSpinBox(this);
    m_spinBox->setObjectName(QStringLiteral("seedSpinBox"));
    m_spinBox->setRange(kMinSeed, kMaxSeed);
    m_spinBox->setValue(kDefaultSeed);
    m_spinBox->setToolTip(
        QStringLiteral("Deterministic seed for reproducible demos."));
    layout->addWidget(m_spinBox);
    layout->addStretch(1);

    setContext(context);
}

void SeedControl::setContext(DemoAppContext *context)
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
    m_spinBox->setValue(m_context->seed());

    // Propagate shared-state changes into the spin box.
    connect(m_context, &DemoAppContext::seedChanged, this,
            [this](int seed) { m_spinBox->setValue(seed); });

    // Write local edits back into the shared context (the change-only setter
    // makes the echo from seedChanged a no-op, so there is no loop).
    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int seed) { m_context->setSeed(seed); });
}

int SeedControl::seed() const
{
    return m_spinBox != nullptr ? m_spinBox->value() : 0;
}
