// PatchOrchestrator — Sprint 30 (D6) config validation implementation.
//
// See config_validator.hpp for the contract. The validator is a pure function:
// it takes the raw config values, checks each rule, and returns a Result that
// reports overall validity plus a per-field human-readable message.

#include "config_validator.hpp"

ConfigValidator::Result ConfigValidator::validate(int fleetSize, double failureRate,
                                                  int seed)
{
    Result result;

    // Fleet size must be at least one endpoint (a fleet of zero is meaningless).
    if (fleetSize < 1) {
        result.valid = false;
        result.fleetSizeError = QStringLiteral("fleet size must be ≥ 1");
    }

    // Failure rate is a fraction and must lie in [0.0, 1.0].
    if (failureRate < 0.0 || failureRate > 1.0) {
        result.valid = false;
        result.failureRateError =
            QStringLiteral("failure rate must be between 0 and 1");
    }

    // Seed must be an integer. The parameter type is `int`, so any value that
    // reaches this function is already an integer; the rule is kept explicit
    // for completeness and consistency with the sprint requirements. A seed is
    // therefore always valid and seedError is never populated.

    // Silence an unused-parameter warning for the always-valid seed check.
    (void)seed;

    return result;
}

QStringList ConfigValidator::Result::errors() const
{
    QStringList list;
    if (!fleetSizeError.isEmpty())
        list << fleetSizeError;
    if (!failureRateError.isEmpty())
        list << failureRateError;
    if (!seedError.isEmpty())
        list << seedError;
    return list;
}

QString ConfigValidator::Result::firstError() const
{
    const QStringList list = errors();
    return list.isEmpty() ? QString() : list.first();
}
