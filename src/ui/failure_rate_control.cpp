// PatchOrchestrator — Sprint 26 (D2) failure rate config implementation.

#include "failure_rate_control.hpp"
#include "demo_app_context.hpp"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

namespace {

// The failure rate is a fraction in [0.0, 1.0]. The slider uses integer
// percent steps (0–100) and the spin box shows the decimal fraction.
const double kMinFailureRate = 0.0;
const double kMaxFailureRate = 1.0;
const double kDefaultFailureRate = 0.0;
const double kStep = 0.01;
const int kSliderMax = 100;

}  // namespace

FailureRateControl::FailureRateControl(DemoAppContext *context, QWidget *parent)
    : QWidget(parent)
    , m_slider(nullptr)
    , m_spinBox(nullptr)
    , m_context(nullptr)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(QStringLiteral("Failure rate:"), this));

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setObjectName(QStringLiteral("failureRateSlider"));
    m_slider->setRange(0, kSliderMax);
    m_slider->setValue(static_cast<int>(kDefaultFailureRate * kSliderMax));
    m_slider->setToolTip(
        QStringLiteral("Per-endpoint failure rate (0.0–1.0) before simulation."));
    layout->addWidget(m_slider, 1);

    m_spinBox = new QDoubleSpinBox(this);
    m_spinBox->setObjectName(QStringLiteral("failureRateSpinBox"));
    m_spinBox->setRange(kMinFailureRate, kMaxFailureRate);
    m_spinBox->setDecimals(2);
    m_spinBox->setSingleStep(kStep);
    m_spinBox->setValue(kDefaultFailureRate);
    m_spinBox->setToolTip(
        QStringLiteral("Per-endpoint failure rate (0.0–1.0) before simulation."));
    layout->addWidget(m_spinBox);

    setContext(context);
}

void FailureRateControl::setContext(DemoAppContext *context)
{
    // Disconnect any previous binding so rebinding does not double-fire.
    if (m_context != nullptr) {
        disconnect(m_context, nullptr, this, nullptr);
        disconnect(m_slider, nullptr, this, nullptr);
        disconnect(m_spinBox, nullptr, this, nullptr);
    }

    m_context = context;
    if (m_context == nullptr)
        return;

    // Read the initial value from the shared context.
    const double rate = m_context->failureRate();
    m_spinBox->setValue(rate);
    m_slider->setValue(static_cast<int>(rate * kSliderMax));

    // Propagate shared-state changes into the control.
    connect(m_context, &DemoAppContext::failureRateChanged, this,
            [this](double value) {
                m_spinBox->setValue(value);
                m_slider->setValue(static_cast<int>(value * kSliderMax));
            });

    // Keep the slider and spin box in sync with each other.
    connect(m_slider, &QSlider::valueChanged, this,
            [this](int percent) { m_spinBox->setValue(percent / 100.0); });
    connect(m_spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double value) {
                m_slider->setValue(static_cast<int>(value * kSliderMax));
                // Write local edits back into the shared context (the
                // change-only setter makes the echo from failureRateChanged a
                // no-op, so there is no loop).
                if (m_context != nullptr)
                    m_context->setFailureRate(value);
            });
}

double FailureRateControl::failureRate() const
{
    return m_spinBox != nullptr ? m_spinBox->value() : 0.0;
}
