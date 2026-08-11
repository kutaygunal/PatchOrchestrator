using System.Text.Json.Serialization;

namespace PatchOrchestrator.Api;

/// <summary>
/// A server-side record of a single operator action (Sprint 32 / E1). Captures
/// the action performed, its target, when it occurred, and the resulting state.
/// Implemented as a C# positional record so it serializes cleanly with
/// System.Text.Json and round-trips through serialization/deserialization.
/// </summary>
/// <remarks>
/// The action is one of "schedule", "pause", "resume", "rollback". The target is
/// the schedule/endpoint id the action applied to. Timestamp is an ISO-8601
/// instant. Result is the outcome (e.g. "ok"/"error") or the resulting state.
/// </remarks>
public record ActionLogEntry(
    string Action,
    string Target,
    DateTimeOffset Timestamp,
    string Result);
