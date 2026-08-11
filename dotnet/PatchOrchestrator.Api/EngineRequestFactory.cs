namespace PatchOrchestrator.Api;

/// <summary>
/// Builds an <see cref="EngineRequest"/> for a live <see cref="EngineSession"/>
/// from configured demo values (Sprint 31 / D7). Turns a configured fleet size,
/// per-endpoint failure rate, and deterministic seed into an engine request with
/// <c>fleetSize</c> endpoints (<c>ep-1</c> … <c>ep-N</c>), each using the given
/// failure rate and the given seed.
/// </summary>
public static class EngineRequestFactory
{
    /// <summary>
    /// Builds an <see cref="EngineRequest"/> from configured values. Rejects an
    /// invalid configuration (fleet size &lt; 1, failure rate outside 0–1) before
    /// any request is produced.
    /// </summary>
    /// <exception cref="ArgumentOutOfRangeException">
    /// If <paramref name="fleetSize"/> is less than 1, or
    /// <paramref name="failureRate"/> is NaN or outside the inclusive [0, 1]
    /// range.
    /// </exception>
    public static EngineRequest Build(int fleetSize, double failureRate, int seed)
    {
        if (fleetSize < 1)
        {
            throw new ArgumentOutOfRangeException(
                nameof(fleetSize), fleetSize, "Fleet size must be at least 1.");
        }

        if (double.IsNaN(failureRate) || failureRate < 0.0 || failureRate > 1.0)
        {
            throw new ArgumentOutOfRangeException(
                nameof(failureRate), failureRate, "Failure rate must be within [0, 1].");
        }

        var endpoints = Enumerable
            .Range(1, fleetSize)
            .Select(i => new EngineEndpointRequest($"ep-{i}", failureRate))
            .ToList();

        return new EngineRequest(endpoints, seed);
    }
}
