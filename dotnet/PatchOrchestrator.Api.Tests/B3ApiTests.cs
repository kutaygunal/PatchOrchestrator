using System.Net;
using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.AspNetCore.Mvc.Testing;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Sprint 13 (B3) integration tests: the live control endpoints
/// POST /api/schedules/{id}/pause|resume|rollback must invoke the live
/// <see cref="EngineSession"/> (not just return a status string), reflect the
/// mutated engine state, and return correct HTTP responses.
/// </summary>
public class B3ApiTests
{
    // Each test uses its own WebApplicationFactory so the shared persistent
    // Python bridge subprocess and in-memory stores are fully isolated between
    // tests (a "run" from the simulate endpoint would otherwise reset the
    // shared bridge rollout used by other tests).

    // ---- T1: Endpoints invoke the session ----

    [Fact]
    public async Task B3_InvokeSession_EndpointsInvokeLiveSession()
    {
        using var factory = new B3ApiFactory();
        var client = factory.CreateClient();

        await CreateScheduleAsync(client, "invoke-1");

        var (c1, s1) = await PostControlAsync(client, "invoke-1", "pause");
        Assert.Equal(HttpStatusCode.OK, c1);
        Assert.Equal("paused", s1);

        var (c2, s2) = await PostControlAsync(client, "invoke-1", "resume");
        Assert.Equal(HttpStatusCode.OK, c2);
        Assert.Equal("running", s2);

        var (c3, s3) = await PostControlAsync(client, "invoke-1", "rollback");
        Assert.Equal(HttpStatusCode.OK, c3);
        Assert.Equal("rolled_back", s3);
    }

    // ---- T2: State changes reflected ----

    [Fact]
    public async Task B3_StateReflected_StateChangesReflectedInStatus()
    {
        using var factory = new B3ApiFactory();
        var client = factory.CreateClient();

        await CreateScheduleAsync(client, "state-1");

        await PostControlAsync(client, "state-1", "pause");
        Assert.Equal("paused", await GetStatusAsync(client, "state-1"));

        await PostControlAsync(client, "state-1", "resume");
        Assert.Equal("running", await GetStatusAsync(client, "state-1"));

        await PostControlAsync(client, "state-1", "rollback");
        Assert.Equal("rolled_back", await GetStatusAsync(client, "state-1"));
    }

    // ---- T3: Correct HTTP responses ----

    [Fact]
    public async Task B3_HttpResponses_CorrectHttpResponses()
    {
        using var factory = new B3ApiFactory();
        var client = factory.CreateClient();

        await CreateScheduleAsync(client, "http-1");

        // Successful control returns 200 with the new state.
        var (c1, s1) = await PostControlAsync(client, "http-1", "pause");
        Assert.Equal(HttpStatusCode.OK, c1);
        Assert.Equal("paused", s1);

        // Unknown schedule id returns 404 Not Found.
        var (c2, _) = await PostControlAsync(client, "does-not-exist", "pause");
        Assert.Equal(HttpStatusCode.NotFound, c2);

        // Invalid transition (pause when already paused) is handled gracefully
        // with a defined 200 response, not a 500 crash.
        var (c3, s3) = await PostControlAsync(client, "http-1", "pause");
        Assert.Equal(HttpStatusCode.OK, c3);
        Assert.Equal("paused", s3);
    }

    // ---- T4: Regression - existing endpoints still work ----

    [Fact]
    public async Task B3_Regression_ExistingEndpointsStillWork()
    {
        using var factory = new B3ApiFactory();
        var client = factory.CreateClient();

        // Health.
        var health = await client.GetAsync("/api/health");
        Assert.Equal(HttpStatusCode.OK, health.StatusCode);

        // Create schedule.
        await CreateScheduleAsync(client, "reg-1");

        // Simulate (drives the Python engine through the bridge).
        var sim = await client.PostAsJsonAsync("/api/schedules/reg-1/simulate", new
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
}

/// <summary>
/// Bootstraps the real API host (via <c>Program</c>) and points the engine
/// bridge at the repo's <c>python/</c> directory regardless of the test
/// runner's working directory.
/// </summary>
public class B3ApiFactory : WebApplicationFactory<Program>
{
    public B3ApiFactory()
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
