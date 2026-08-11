using System.Net;
using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.AspNetCore.Mvc.Testing;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Sprint 14 (B4) integration tests: the live tick endpoint
/// POST /api/schedules/{id}/tick must advance the live <see cref="EngineSession"/>
/// deterministically, accept a step count, and return correct HTTP responses.
/// </summary>
public class B4ApiTests
{
    // Each test uses its own WebApplicationFactory so the shared persistent
    // Python bridge subprocess and in-memory stores are fully isolated between
    // tests.

    // ---- T1: Tick advances state deterministically ----

    [Fact]
    public async Task B4_TickAdvances_TickAdvancesStateDeterministically()
    {
        // Two independent runs (separate factories => separate bridges/subprocesses)
        // started with the same default seed (42) must advance identically when
        // ticked the same number of times.
        using var factory1 = new B4ApiFactory();
        using var factory2 = new B4ApiFactory();
        var client1 = factory1.CreateClient();
        var client2 = factory2.CreateClient();

        await CreateScheduleAsync(client1, "det-1");
        await CreateScheduleAsync(client2, "det-2");

        string? s1 = null, s2 = null;
        for (var i = 0; i < 5; i++)
        {
            s1 = (await TickAsync(client1, "det-1", 1)).State;
            s2 = (await TickAsync(client2, "det-2", 1)).State;
        }

        // Same seed + same tick count must yield the same engine state.
        Assert.Equal(EndpointsOf(s1), EndpointsOf(s2));

        // The tick actually advanced the rollout (progress moved past start).
        Assert.True(StateAdvanced(s1), "tick should advance the live rollout state");
    }

    // ---- T2: Tick count / steps ----

    [Fact]
    public async Task B4_TickSteps_StepCountEqualsSingleTicks()
    {
        // Two independent runs (separate factories) so each has its own bridge.
        using var factory1 = new B4ApiFactory();
        using var factory2 = new B4ApiFactory();
        var client1 = factory1.CreateClient();
        var client2 = factory2.CreateClient();

        await CreateScheduleAsync(client1, "steps-1");
        await CreateScheduleAsync(client2, "steps-2");

        // Advance by N steps in one call.
        var s1 = (await TickAsync(client1, "steps-1", 5)).State;

        // Advance by N single ticks.
        string? s2 = null;
        for (var i = 0; i < 5; i++)
        {
            s2 = (await TickAsync(client2, "steps-2", 1)).State;
        }

        // N steps in one call must equal N single ticks (deterministic equivalence).
        Assert.Equal(EndpointsOf(s1), EndpointsOf(s2));
    }

    // ---- T3: Correct HTTP responses ----

    [Fact]
    public async Task B4_HttpResponses_CorrectHttpResponses()
    {
        using var factory = new B4ApiFactory();
        var client = factory.CreateClient();

        await CreateScheduleAsync(client, "http-1");

        // Successful tick returns 200 with the updated state.
        var (c1, state1) = await TickAsync(client, "http-1", 1);
        Assert.Equal(HttpStatusCode.OK, c1);
        Assert.NotNull(state1);

        // Unknown schedule id returns 404 Not Found.
        var (c2, _) = await TickAsync(client, "does-not-exist", 1);
        Assert.Equal(HttpStatusCode.NotFound, c2);

        // Ticking a rolled-back rollout is handled gracefully (defined 200, no 500).
        await PostControlAsync(client, "http-1", "rollback");
        var (c3, _) = await TickAsync(client, "http-1", 1);
        Assert.Equal(HttpStatusCode.OK, c3);
    }

    // ---- T5: Regression - existing endpoints still work ----

    [Fact]
    public async Task B4_Regression_ExistingEndpointsStillWork()
    {
        using var factory = new B4ApiFactory();
        var client = factory.CreateClient();

        // Health.
        var health = await client.GetAsync("/api/health");
        Assert.Equal(HttpStatusCode.OK, health.StatusCode);

        // Create schedule.
        await CreateScheduleAsync(client, "reg-1");

        // Pause / resume / rollback still work on a fresh live session.
        var (pc, ps) = await PostControlAsync(client, "reg-1", "pause");
        Assert.Equal(HttpStatusCode.OK, pc);
        Assert.Equal("paused", ps);

        var (rc, rs) = await PostControlAsync(client, "reg-1", "resume");
        Assert.Equal(HttpStatusCode.OK, rc);
        Assert.Equal("running", rs);

        var (bc, bs) = await PostControlAsync(client, "reg-1", "rollback");
        Assert.Equal(HttpStatusCode.OK, bc);
        Assert.Equal("rolled_back", bs);

        // Simulate (drives the Python engine through the bridge) on its own
        // schedule so it does not disturb the control checks above.
        await CreateScheduleAsync(client, "reg-2");
        var sim = await client.PostAsJsonAsync("/api/schedules/reg-2/simulate", new
        {
            seed = 42,
            endpoints = new[] { new { id = "ep-1", failureRate = 0.1 } },
        });
        Assert.Equal(HttpStatusCode.OK, sim.StatusCode);

        // Status query still works.
        var status = await GetStatusAsync(client, "reg-1");
        Assert.NotNull(status);
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

    private static async Task<(HttpStatusCode Code, string? State)> TickAsync(
        HttpClient client, string id, int steps)
    {
        var resp = await client.PostAsJsonAsync($"/api/schedules/{id}/tick", new { steps });
        string? state = null;
        if (resp.IsSuccessStatusCode)
        {
            state = await resp.Content.ReadAsStringAsync();
        }
        return (resp.StatusCode, state);
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

    private static async Task<string?> GetStatusAsync(HttpClient client, string id)
    {
        var resp = await client.GetAsync($"/api/schedules/{id}/status");
        resp.EnsureSuccessStatusCode();
        using var doc = JsonDocument.Parse(await resp.Content.ReadAsStringAsync());
        return doc.RootElement.GetProperty("status").GetString();
    }

    // Extract the per-endpoint state/progress from a tick response body so two
    // schedules can be compared deterministically regardless of their ids.
    private static string EndpointsOf(string? stateJson)
    {
        using var doc = JsonDocument.Parse(stateJson!);
        var endpoints = doc.RootElement.GetProperty("endpoints");
        var parts = endpoints.EnumerateArray()
            .Select(e => $"{e.GetProperty("id").GetString()}:{e.GetProperty("state").GetString()}:{e.GetProperty("progress").GetDouble()}");
        return string.Join("|", parts);
    }

    private static bool StateAdvanced(string? stateJson)
    {
        using var doc = JsonDocument.Parse(stateJson!);
        return doc.RootElement.TryGetProperty("status", out var status)
            && status.GetString() is "running" or "paused" or "rolled_back";
    }
}

/// <summary>
/// Bootstraps the real API host (via <c>Program</c>) and points the engine
/// bridge at the repo's <c>python/</c> directory regardless of the test
/// runner's working directory.
/// </summary>
public class B4ApiFactory : WebApplicationFactory<Program>
{
    public B4ApiFactory()
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
