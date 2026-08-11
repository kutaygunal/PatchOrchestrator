using System.Net;
using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.AspNetCore.Mvc.Testing;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Sprint 34 (E3) integration tests for operator action recording. Every
/// schedule/pause/resume/rollback/tick operation must append an E1
/// <see cref="ActionLogEntry"/> (action, target = schedule id,
/// timestamp = <see cref="DateTimeOffset.UtcNow"/>, result = resulting state)
/// to the schedule's <c>Actions</c> collection so the E2
/// GET /api/schedules/{id}/actions endpoint returns a complete, chronological
/// log.
/// </summary>
public class E3ActionRecordingTests
{
    // Each test uses its own WebApplicationFactory so the shared persistent
    // Python bridge subprocess and in-memory stores are fully isolated.

    // ---- T1: Actions recorded on operations ----

    [Fact]
    public async Task E3_Recorded_EveryOperationRecordsActionWithCorrectFields()
    {
        using var factory = new E3ApiFactory();
        var client = factory.CreateClient();

        // schedule create + pause + resume + rollback = 4 operations.
        await CreateScheduleAsync(client, "recorded-1");
        await PostControlAsync(client, "recorded-1", "pause");
        await PostControlAsync(client, "recorded-1", "resume");
        await PostControlAsync(client, "recorded-1", "rollback");

        var actions = await GetActionsAsync(client, "recorded-1");

        // Each operation recorded exactly one action entry.
        Assert.Equal(4, actions.Count);

        // Correct action type for each operation, in performed order.
        Assert.Equal(new[] { "schedule", "pause", "resume", "rollback" },
            actions.Select(a => a.Action).ToArray());

        // Correct target (the schedule id) for every entry.
        Assert.All(actions, a => Assert.Equal("recorded-1", a.Target));

        // Correct result = the resulting state (schedule -> "ok",
        // pause -> "paused", resume -> "running", rollback -> "rolled_back").
        Assert.Equal(new[] { "ok", "paused", "running", "rolled_back" },
            actions.Select(a => a.Result).ToArray());

        // Every entry has a valid (parseable) timestamp.
        Assert.All(actions, a =>
            Assert.True(DateTimeOffset.TryParse(a.Timestamp.ToString("O"), out _),
                "Every action entry must have a valid timestamp"));

        // Timestamps are present (not default).
        Assert.All(actions, a =>
            Assert.True(a.Timestamp != default,
                "Every action entry must carry a real timestamp"));
    }

    // ---- T2: Complete log ----

    [Fact]
    public async Task E3_CompleteLog_LogContainsAllOperationsInOrder()
    {
        using var factory = new E3ApiFactory();
        var client = factory.CreateClient();

        // Perform schedule create + a full sequence of control operations,
        // plus a tick, so the log covers every recorded action type.
        var performed = new List<string> { "schedule" };
        await CreateScheduleAsync(client, "complete-1");
        foreach (var op in new[] { "pause", "resume", "pause", "resume", "rollback" })
        {
            await PostControlAsync(client, "complete-1", op);
            performed.Add(op);
        }
        await client.PostAsync("/api/schedules/complete-1/tick", null);
        performed.Add("tick");

        var actions = await GetActionsAsync(client, "complete-1");

        // Count matches the number of operations exactly.
        Assert.Equal(performed.Count, actions.Count);

        // No operations missing and order is preserved.
        Assert.Equal(performed, actions.Select(a => a.Action).ToArray());

        // Complete log is in chronological order (non-decreasing timestamps).
        var timestamps = actions.Select(a => a.Timestamp).ToList();
        for (var i = 1; i < timestamps.Count; i++)
        {
            Assert.True(timestamps[i] >= timestamps[i - 1],
                "The complete log must preserve chronological order");
        }

        // Every entry references the same target schedule.
        Assert.All(actions, a => Assert.Equal("complete-1", a.Target));
    }

    // ---- T3: Regression - existing B3/E1/E2 behavior still works ----

    [Fact]
    public async Task E3_Regression_ExistingB3E1E2BehaviorStillWorks()
    {
        using var factory = new E3ApiFactory();
        var client = factory.CreateClient();

        // B3: live control endpoints still mutate engine state.
        await CreateScheduleAsync(client, "reg-e3");
        var (c1, s1) = await PostControlAsync(client, "reg-e3", "pause");
        Assert.Equal(HttpStatusCode.OK, c1);
        Assert.Equal("paused", s1);

        var (c2, s2) = await PostControlAsync(client, "reg-e3", "resume");
        Assert.Equal(HttpStatusCode.OK, c2);
        Assert.Equal("running", s2);

        var (c3, s3) = await PostControlAsync(client, "reg-e3", "rollback");
        Assert.Equal(HttpStatusCode.OK, c3);
        Assert.Equal("rolled_back", s3);

        // Unknown schedule id still returns 404.
        var (c4, _) = await PostControlAsync(client, "does-not-exist", "pause");
        Assert.Equal(HttpStatusCode.NotFound, c4);

        // E2: the action log endpoint still returns the recorded actions.
        var actions = await GetActionsAsync(client, "reg-e3");
        Assert.Equal(new[] { "schedule", "pause", "resume", "rollback" },
            actions.Select(a => a.Action).ToArray());

        // E2: unknown schedule returns 404 for the actions endpoint too.
        var unknown = await client.GetAsync("/api/schedules/does-not-exist/actions");
        Assert.Equal(HttpStatusCode.NotFound, unknown.StatusCode);

        // E1: ActionLogEntry still round-trips through System.Text.Json.
        var entry = new ActionLogEntry("resume", "reg-e3", DateTimeOffset.Now, "running");
        var roundTrip = JsonSerializer.Deserialize<ActionLogEntry>(
            JsonSerializer.Serialize(entry));
        Assert.Equal(entry, roundTrip);

        // Health endpoint still works.
        var health = await client.GetAsync("/api/health");
        Assert.Equal(HttpStatusCode.OK, health.StatusCode);
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
}

/// <summary>
/// Bootstraps the real API host (via <c>Program</c>) for the E3 integration
/// tests, pointing the engine bridge at the repo's <c>python/</c> directory.
/// </summary>
public class E3ApiFactory : WebApplicationFactory<Program>
{
    public E3ApiFactory()
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
