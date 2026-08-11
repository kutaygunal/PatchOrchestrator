using System.Net;
using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.AspNetCore.Mvc.Testing;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Sprint 33 (E2) integration tests for the API action log endpoint
/// GET /api/schedules/{id}/actions. The endpoint returns the recorded operator
/// actions (schedule, pause, resume, rollback, tick) in chronological order
/// using the E1 <see cref="ActionLogEntry"/> type. Returns 404 for an unknown
/// schedule id, matching the other endpoints' style.
/// </summary>
public class E2ApiTests
{
    // Each test uses its own WebApplicationFactory so the shared persistent
    // Python bridge subprocess and in-memory stores are fully isolated.

    // ---- T1: Endpoint returns recorded actions ----

    [Fact]
    public async Task E2_ReturnsActions_EndpointReturnsRecordedActions()
    {
        using var factory = new E2ApiFactory();
        var client = factory.CreateClient();

        // Create a schedule (records 1 action) then pause/resume/rollback.
        await CreateScheduleAsync(client, "returns-1");
        await PostControlAsync(client, "returns-1", "pause");
        await PostControlAsync(client, "returns-1", "resume");
        await PostControlAsync(client, "returns-1", "rollback");

        var actions = await GetActionsAsync(client, "returns-1");

        // schedule + pause + resume + rollback = 4 recorded actions.
        Assert.Equal(4, actions.Count);
        Assert.Equal(new[] { "schedule", "pause", "resume", "rollback" },
            actions.Select(a => a.Action).ToArray());
    }

    // ---- T2: Correct format ----

    [Fact]
    public async Task E2_Format_EachActionHasCorrectFieldsInOrder()
    {
        using var factory = new E2ApiFactory();
        var client = factory.CreateClient();

        await CreateScheduleAsync(client, "format-1");
        await PostControlAsync(client, "format-1", "pause");
        await PostControlAsync(client, "format-1", "resume");

        var actions = await GetActionsAsync(client, "format-1");

        // schedule, pause, resume in chronological order.
        Assert.Equal(3, actions.Count);
        Assert.Equal(new[] { "schedule", "pause", "resume" },
            actions.Select(a => a.Action).ToArray());

        // Each entry has the correct fields populated: result is the resulting
        // state for control actions and "ok" for the schedule-creation action.
        Assert.Equal("ok", actions[0].Result);
        Assert.Equal("paused", actions[1].Result);
        Assert.Equal("running", actions[2].Result);
        Assert.All(actions, e =>
        {
            Assert.Equal("format-1", e.Target);
            Assert.True(DateTimeOffset.TryParse(e.Timestamp.ToString("O"), out _));
        });

        // Timestamps are non-decreasing (chronological order).
        var timestamps = actions.Select(a => a.Timestamp).ToList();
        for (var i = 1; i < timestamps.Count; i++)
        {
            Assert.True(timestamps[i] >= timestamps[i - 1],
                "Actions must be in chronological order");
        }
    }

    [Fact]
    public async Task E2_Format_UnknownScheduleReturnsNotFound()
    {
        using var factory = new E2ApiFactory();
        var client = factory.CreateClient();

        var resp = await client.GetAsync("/api/schedules/does-not-exist/actions");
        Assert.Equal(HttpStatusCode.NotFound, resp.StatusCode);
    }

    [Fact]
    public async Task E2_Format_TickActionsAreRecorded()
    {
        using var factory = new E2ApiFactory();
        var client = factory.CreateClient();

        await CreateScheduleAsync(client, "format-tick");
        await client.PostAsync("/api/schedules/format-tick/tick", null);

        var actions = await GetActionsAsync(client, "format-tick");

        // schedule + tick = 2 recorded actions.
        Assert.Equal(2, actions.Count);
        Assert.Equal("schedule", actions[0].Action);
        Assert.Equal("tick", actions[1].Action);
    }

    // ---- T3: Regression - existing endpoints and models still work ----

    [Fact]
    public async Task E2_Regression_ExistingEndpointsAndModelsStillWork()
    {
        using var factory = new E2ApiFactory();
        var client = factory.CreateClient();

        // Health.
        var health = await client.GetAsync("/api/health");
        Assert.Equal(HttpStatusCode.OK, health.StatusCode);

        // Create schedule.
        await CreateScheduleAsync(client, "reg-e2");

        // Existing control endpoints (pause/resume/rollback) still work.
        var (c1, s1) = await PostControlAsync(client, "reg-e2", "pause");
        Assert.Equal(HttpStatusCode.OK, c1);
        Assert.Equal("paused", s1);

        var (c2, s2) = await PostControlAsync(client, "reg-e2", "resume");
        Assert.Equal(HttpStatusCode.OK, c2);
        Assert.Equal("running", s2);

        var (c3, s3) = await PostControlAsync(client, "reg-e2", "rollback");
        Assert.Equal(HttpStatusCode.OK, c3);
        Assert.Equal("rolled_back", s3);

        // Simulate still drives the Python engine through the bridge.
        var sim = await client.PostAsJsonAsync("/api/schedules/reg-e2/simulate", new
        {
            seed = 42,
            endpoints = new[] { new { id = "ep-1", failureRate = 0.1 } },
        });
        Assert.Equal(HttpStatusCode.OK, sim.StatusCode);

        // Status query still works.
        var status = await GetStatusAsync(client, "reg-e2");
        Assert.NotNull(status);

        // E1 ActionLogEntry model still round-trips through System.Text.Json.
        var entry = new ActionLogEntry("resume", "reg-e2", DateTimeOffset.Now, "running");
        var roundTrip = JsonSerializer.Deserialize<ActionLogEntry>(
            JsonSerializer.Serialize(entry));
        Assert.Equal(entry, roundTrip);
    }

    // ---- Helpers ----

    private static async Task CreateScheduleAsync(HttpClient client, string id)
    {
        var resp = await client.PostAsJsonAsync("/api/schedules", new
        {
            id,
            package = "pkg",
            groupId = "grp",
        });
        resp.EnsureSuccessStatusCode();
    }

    private static async Task<(HttpStatusCode Code, string? Status)> PostControlAsync(
        HttpClient client, string id, string action)
    {
        var resp = await client.PostAsync($"/api/schedules/{id}/{action}", null);
        var body = await resp.Content.ReadAsStringAsync();
        string? status = null;
        if (resp.IsSuccessStatusCode)
        {
            using var doc = JsonDocument.Parse(body);
            status = doc.RootElement.GetProperty("status").GetString();
        }
        return (resp.StatusCode, status);
    }

    private static async Task<List<ActionLogEntry>> GetActionsAsync(HttpClient client, string id)
    {
        var resp = await client.GetAsync($"/api/schedules/{id}/actions");
        resp.EnsureSuccessStatusCode();
        var json = await resp.Content.ReadAsStringAsync();
        // The API serializes with camelCase naming (ASP.NET default), so match
        // it case-insensitively against the PascalCase record properties.
        var options = new JsonSerializerOptions { PropertyNameCaseInsensitive = true };
        return JsonSerializer.Deserialize<List<ActionLogEntry>>(json, options) ?? new List<ActionLogEntry>();
    }

    private static async Task<string?> GetStatusAsync(HttpClient client, string id)
    {
        var resp = await client.GetAsync($"/api/schedules/{id}/status");
        resp.EnsureSuccessStatusCode();
        using var doc = JsonDocument.Parse(await resp.Content.ReadAsStringAsync());
        return doc.RootElement.GetProperty("status").GetString();
    }
}

/// <summary>
/// Bootstraps the real API host (via <c>Program</c>) for the E2 integration
/// tests, pointing the engine bridge at the repo's <c>python/</c> directory.
/// </summary>
public class E2ApiFactory : WebApplicationFactory<Program>
{
    public E2ApiFactory()
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
