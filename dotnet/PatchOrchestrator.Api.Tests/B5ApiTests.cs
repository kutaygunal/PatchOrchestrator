using System.Net;
using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.AspNetCore.Mvc.Testing;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Sprint 15 (B5) integration tests: the live status stream endpoint
/// GET /api/schedules/{id}/status/stream (SSE) must deliver live state changes
/// to connected clients, emit on state change, and return correct HTTP
/// behavior (content type, 404 for unknown id, graceful client disconnect).
/// </summary>
public class B5ApiTests
{
    // Each test uses its own WebApplicationFactory so the shared persistent
    // Python bridge subprocess and in-memory stores are fully isolated between
    // tests.

    // ---- T1: Stream emits on state change ----

    [Fact]
    public async Task B5_StreamEmits_StreamEmitsOnStateChange()
    {
        using var factory = new B5ApiFactory();
        var client = factory.CreateClient();
        await CreateScheduleAsync(client, "emit-1");

        // Open the status stream (headers-first so we can read events live).
        using var streamResp = await client.GetAsync(
            "/api/schedules/emit-1/status/stream", HttpCompletionOption.ResponseHeadersRead);
        Assert.Equal(HttpStatusCode.OK, streamResp.StatusCode);
        Assert.Equal("text/event-stream", streamResp.Content.Headers.ContentType?.MediaType);

        var reader = new StreamReader(await streamResp.Content.ReadAsStreamAsync());

        // The stream opens with a baseline event reflecting the current state.
        var baseline = await ReadEventAsync(reader);
        Assert.NotNull(baseline);

        // Trigger a state change (pause).
        var (c, s) = await PostControlAsync(client, "emit-1", "pause");
        Assert.Equal(HttpStatusCode.OK, c);
        Assert.Equal("paused", s);

        // The stream emits an event reflecting the new state.
        var evt = await ReadEventAsync(reader);
        Assert.NotNull(evt);
        Assert.Equal("paused", evt.Status);

        // The stream stays open between events (no premature close).
        Assert.True(await IsStreamOpenAsync(reader));
    }

    // ---- T2: Client receives updates ----

    [Fact]
    public async Task B5_ClientReceives_ClientReceivesUpdatesInOrder()
    {
        using var factory = new B5ApiFactory();
        var client = factory.CreateClient();
        await CreateScheduleAsync(client, "recv-1");

        using var streamResp = await client.GetAsync(
            "/api/schedules/recv-1/status/stream", HttpCompletionOption.ResponseHeadersRead);
        var reader = new StreamReader(await streamResp.Content.ReadAsStreamAsync());

        // Consume the baseline event.
        await ReadEventAsync(reader);

        // Trigger multiple sequential state changes.
        await PostControlAsync(client, "recv-1", "pause");
        await PostControlAsync(client, "recv-1", "resume");
        await PostControlAsync(client, "recv-1", "rollback");

        // The client receives each update in order.
        var e1 = await ReadEventAsync(reader);
        var e2 = await ReadEventAsync(reader);
        var e3 = await ReadEventAsync(reader);

        Assert.Equal("paused", e1!.Status);
        Assert.Equal("running", e2!.Status);
        Assert.Equal("rolled_back", e3!.Status);

        // The client can keep reading the stream (still open).
        Assert.True(await IsStreamOpenAsync(reader));
    }

    // ---- T3: Correct HTTP behavior ----

    [Fact]
    public async Task B5_HttpBehavior_CorrectHttpBehavior()
    {
        using var factory = new B5ApiFactory();
        var client = factory.CreateClient();

        // Unknown schedule id returns 404 Not Found.
        var notFound = await client.GetAsync("/api/schedules/nope/status/stream");
        Assert.Equal(HttpStatusCode.NotFound, notFound.StatusCode);

        // Known schedule returns text/event-stream.
        await CreateScheduleAsync(client, "http-1");
        using var streamResp = await client.GetAsync(
            "/api/schedules/http-1/status/stream", HttpCompletionOption.ResponseHeadersRead);
        Assert.Equal(HttpStatusCode.OK, streamResp.StatusCode);
        Assert.Equal("text/event-stream", streamResp.Content.Headers.ContentType?.MediaType);

        // Client disconnect is handled gracefully: dispose the stream, then the
        // API still serves other requests (no crash, no orphaned task).
        var reader = new StreamReader(await streamResp.Content.ReadAsStreamAsync());
        await ReadEventAsync(reader); // baseline
        streamResp.Dispose(); // simulate client disconnect

        var health = await client.GetAsync("/api/health");
        Assert.Equal(HttpStatusCode.OK, health.StatusCode);
    }

    // ---- T4: Regression - existing endpoints still work ----

    [Fact]
    public async Task B5_Regression_ExistingEndpointsStillWork()
    {
        using var factory = new B5ApiFactory();
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

        // Tick still works.
        var (tc, _) = await TickAsync(client, "reg-1", 1);
        Assert.Equal(HttpStatusCode.OK, tc);

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

    private static async Task<string?> GetStatusAsync(HttpClient client, string id)
    {
        var resp = await client.GetAsync($"/api/schedules/{id}/status");
        resp.EnsureSuccessStatusCode();
        using var doc = JsonDocument.Parse(await resp.Content.ReadAsStringAsync());
        return doc.RootElement.GetProperty("status").GetString();
    }

    // Read one SSE event from the stream: skip non-data lines (blank separators)
    // and return the parsed payload. Returns null if the stream closes.
    private static async Task<StreamEvent?> ReadEventAsync(StreamReader reader)
    {
        string? data = null;
        while (true)
        {
            var line = await reader.ReadLineAsync();
            if (line == null)
            {
                return null; // stream closed
            }
            if (line.StartsWith("data:"))
            {
                data = line.Substring(5).Trim();
                break;
            }
        }

        if (data == null)
        {
            return null;
        }

        using var doc = JsonDocument.Parse(data);
        return new StreamEvent(
            doc.RootElement.GetProperty("id").GetString(),
            doc.RootElement.GetProperty("status").GetString());
    }

    // True if the stream is still open: a read that times out waiting for data
    // means the stream is alive (no premature close), whereas a null read means
    // the underlying stream has closed.
    private static async Task<bool> IsStreamOpenAsync(StreamReader reader)
    {
        using var cts = new CancellationTokenSource(TimeSpan.FromMilliseconds(300));
        try
        {
            var line = await reader.ReadLineAsync(cts.Token);
            return line != null;
        }
        catch (OperationCanceledException)
        {
            // Read timed out waiting for data => stream is still open.
            return true;
        }
    }

    private sealed record StreamEvent(string? Id, string? Status);
}

/// <summary>
/// Bootstraps the real API host (via <c>Program</c>) and points the engine
/// bridge at the repo's <c>python/</c> directory regardless of the test
/// runner's working directory.
/// </summary>
public class B5ApiFactory : WebApplicationFactory<Program>
{
    public B5ApiFactory()
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
