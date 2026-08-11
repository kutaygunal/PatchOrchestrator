using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.AspNetCore.Mvc.Testing;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Sprint 32 (E1) tests for the server-side <see cref="ActionLogEntry"/> model:
/// a record of one operator action (schedule, pause, resume, rollback) with its
/// target, timestamp, and result. Implemented as a positional record so it
/// round-trips cleanly through System.Text.Json.
/// </summary>
public class E1ActionLogModelTests
{
    // ---- T1: Field access ----

    [Fact]
    public void E1_FieldAccess_AllFieldsSetAndRetrievedCorrectly()
    {
        var timestamp = new DateTimeOffset(2025, 8, 10, 14, 30, 0, TimeSpan.Zero);
        var entry = new ActionLogEntry(
            Action: "pause",
            Target: "sch-100",
            Timestamp: timestamp,
            Result: "ok");

        // Each field is set to the value passed to the constructor.
        Assert.Equal("pause", entry.Action);
        Assert.Equal("sch-100", entry.Target);
        Assert.Equal(timestamp, entry.Timestamp);
        Assert.Equal("ok", entry.Result);
    }

    [Fact]
    public void E1_FieldAccess_EachActionKindPopulatesFieldsCorrectly()
    {
        var schedule = new ActionLogEntry("schedule", "sch-1", DateTimeOffset.Now, "ok");
        var pause = new ActionLogEntry("pause", "sch-1", DateTimeOffset.Now, "paused");
        var resume = new ActionLogEntry("resume", "sch-1", DateTimeOffset.Now, "running");
        var rollback = new ActionLogEntry("rollback", "sch-1", DateTimeOffset.Now, "rolled_back");

        Assert.Equal("schedule", schedule.Action);
        Assert.Equal("pause", pause.Action);
        Assert.Equal("resume", resume.Action);
        Assert.Equal("rollback", rollback.Action);

        Assert.Equal("ok", schedule.Result);
        Assert.Equal("paused", pause.Result);
        Assert.Equal("running", resume.Result);
        Assert.Equal("rolled_back", rollback.Result);

        Assert.All(new[] { schedule, pause, resume, rollback }, e => Assert.Equal("sch-1", e.Target));
    }

    // ---- T2: Serialization ----

    [Fact]
    public void E1_Serialization_AllFieldsPreservedThroughJsonRoundTrip()
    {
        var timestamp = new DateTimeOffset(2025, 8, 10, 14, 30, 45, TimeSpan.FromHours(2));
        var original = new ActionLogEntry("resume", "sch-42", timestamp, "running");

        var json = JsonSerializer.Serialize(original);
        var deserialized = JsonSerializer.Deserialize<ActionLogEntry>(json);

        Assert.NotNull(deserialized);
        Assert.Equal(original, deserialized);
        Assert.Equal(original.Action, deserialized.Action);
        Assert.Equal(original.Target, deserialized.Target);
        Assert.Equal(original.Timestamp, deserialized.Timestamp);
        Assert.Equal(original.Result, deserialized.Result);
    }

    [Fact]
    public void E1_Serialization_ProducesValidJsonWithExpectedPropertyNames()
    {
        var entry = new ActionLogEntry("schedule", "sch-7", DateTimeOffset.Now, "ok");

        var json = JsonSerializer.Serialize(entry);
        using var doc = JsonDocument.Parse(json); // must be valid JSON

        var root = doc.RootElement;
        Assert.Equal("schedule", root.GetProperty("Action").GetString());
        Assert.Equal("sch-7", root.GetProperty("Target").GetString());
        Assert.Equal("ok", root.GetProperty("Result").GetString());
        Assert.True(root.TryGetProperty("Timestamp", out var tsProp));
        Assert.True(tsProp.ValueKind is JsonValueKind.String);
    }

    [Fact]
    public void E1_Serialization_TimestampSerializesAsIso8601()
    {
        var entry = new ActionLogEntry("pause", "sch-1", DateTimeOffset.Now, "paused");

        var json = JsonSerializer.Serialize(entry);
        using var doc = JsonDocument.Parse(json);
        var ts = doc.RootElement.GetProperty("Timestamp").GetString();

        Assert.NotNull(ts);
        // System.Text.Json emits ISO-8601 (round-trip "O") format.
        Assert.DoesNotContain(' ', ts!);
        Assert.Equal(entry.Timestamp, DateTimeOffset.Parse(ts!));
    }

    // ---- T3: Regression (A3 / backend still works) ----

    [Fact]
    public async Task E1_Regression_BackendHostStillBootsAndServesHealth()
    {
        using var factory = new E1ApiFactory();
        var client = factory.CreateClient();

        // The web host still boots and the health endpoint responds.
        var health = await client.GetAsync("/api/health");
        health.EnsureSuccessStatusCode();

        // Creating a schedule still works (drives the live engine session).
        var create = await client.PostAsJsonAsync("/api/schedules", new
        {
            id = "e1-regression",
            package = "pkg",
            groupId = "grp",
            fleetSize = 3,
            failureRate = 0.1,
            seed = 42,
        });
        create.EnsureSuccessStatusCode();

        var status = await client.GetAsync("/api/schedules/e1-regression/status");
        status.EnsureSuccessStatusCode();
        var body = await status.Content.ReadAsStringAsync();
        using var doc = JsonDocument.Parse(body);
        Assert.Equal("running", doc.RootElement.GetProperty("status").GetString());
    }

    [Fact]
    public void E1_Regression_ActionLogEntryWorksAlongsideExistingBackendTypes()
    {
        // The action log model is compatible with the existing backend records
        // and can be combined with a live session result without any regression.
        var request = new EngineRequest(
            new List<EngineEndpointRequest> { new("ep-1", 0.1), new("ep-2", 0.0) },
            Seed: 42);

        using var session = new EngineSession(new EngineBridge(), request);

        var log = new ActionLogEntry("resume", "sch-1", DateTimeOffset.Now, "running");
        var sessionState = session.Resume();

        Assert.Equal("running", log.Result);
        Assert.All(sessionState.Endpoints, e => Assert.Equal("running", e.State));
        Assert.Equal(2, sessionState.Endpoints.Count);
    }

    private static string FindPythonDir()
    {
        var dir = new DirectoryInfo(AppContext.BaseDirectory);
        while (dir != null)
        {
            var candidate = Path.Combine(dir.FullName, "python");
            if (Directory.Exists(candidate))
            {
                return candidate;
            }
            dir = dir.Parent;
        }
        throw new DirectoryNotFoundException("Could not locate the python/ directory.");
    }
}

/// <summary>
/// Bootstraps the real API host (via <c>Program</c>) for the E1 regression
/// tests, pointing the engine bridge at the repo's <c>python/</c> directory.
/// </summary>
public class E1ApiFactory : WebApplicationFactory<Program>
{
    public E1ApiFactory()
    {
        Environment.SetEnvironmentVariable("PATCHORCH_PYTHON_DIR", FindPythonDir());
    }

    private static string FindPythonDir()
    {
        var dir = new DirectoryInfo(AppContext.BaseDirectory);
        while (dir != null)
        {
            var candidate = Path.Combine(dir.FullName, "python");
            if (Directory.Exists(candidate))
            {
                return candidate;
            }
            dir = dir.Parent;
        }
        throw new DirectoryNotFoundException("Could not locate the python/ directory.");
    }
}
