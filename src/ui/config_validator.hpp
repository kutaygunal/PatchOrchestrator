// PatchOrchestrator — Sprint 30 (D6) config validation.
//
// A small, self-contained validator for the demo rollout configuration (fleet
// size, failure rate, seed). It is used before a rollout starts to make sure
// the configured values are sane, and it reports a human-readable message per
// offending field so the UI can show an inline error. Kept free of Qt Widgets
// so it is easy to unit-test and reuse.

#ifndef PATCHORCHESTRATOR_UI_CONFIG_VALIDATOR_HPP
#define PATCHORCHESTRATOR_UI_CONFIG_VALIDATOR_HPP

#include <QString>
#include <QStringList>

class ConfigValidator
{
public:
    // The outcome of validating a configuration: whether the whole config is
    // valid, plus a human-readable message for each field (empty when that
    // field is valid). `errors()` collects the non-empty messages and
    // `firstError()` returns the first one, so callers can surface a single
    // inline message near the offending control.
    struct Result
    {
        bool valid = true;
        QString fleetSizeError;
        QString failureRateError;
        QString seedError;

        bool isValid() const { return valid; }

        // All non-empty field errors, in field order.
        QStringList errors() const;

        // The first non-empty field error (or an empty string when valid).
        QString firstError() const;
    };

    // Validate the given config values. A valid config passes; each field is
    // checked independently:
    //   * fleet size must be >= 1,
    //   * failure rate must be in [0.0, 1.0],
    //   * seed must be an integer — the QSpinBox only stores ints, so the
    //     value passed here is always an integer and therefore always valid;
    //     the rule is kept explicit for completeness.
    static Result validate(int fleetSize, double failureRate, int seed);
};

#endif // PATCHORCHESTRATOR_UI_CONFIG_VALIDATOR_HPP
