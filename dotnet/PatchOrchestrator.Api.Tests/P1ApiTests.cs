using System.Net;
using System.Net.Http.Json;
using System.Text.Json;
using Microsoft.AspNetCore.Mvc.Testing;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Phase 1 (P1) integration tests: the API must persist a schedule's fleet
/// configuration (and derived endpoints) on create, and expose a list endpoint
/// <c>GET /api/schedules</c> that returns all known schedules newest-first.
/// These tests define the P1 contract that <c>senior-engineer-p1</c> must satisfy.
/// </summary>
public class P1ApiTests
{
    // Each test uses its own WebApplicationFactory so the in-memory stores and
    // the persistent Python bridge subprocess are fully isolated between tests.

    // ---- T1: Stored fleet on create (configured) ----

    [Fact]
    public async Task P1_StoredFleetConfigured_DerivedEndpointsAndSeedPersisted()
    {
        using var factory = new P1ApiFactory();
        var client = factory.CreateClient();

        var created = await CreateScheduleAsync(client, "cfg-1", fleetSize: 4, failureRate: 0.25, seed: 7);
        Assert.Equal(HttpStatusCode.Created, created);

        var detail = await GetDetailAsync(client, "cfg-1");
        Assert.NotNull(detail);

        // Exactly fleetSize endpoints, named ep-1 … ep-N.
        var fleet = detail!.Fleet;
        Assert.Equal(4, fleet.Count);
        Assert.Equal(new[] { "ep-1", "ep-2", "ep-3", "ep-4" }, fleet.Select(e => e.Id));

        // Each endpoint carries the configured failure rate; the seed is stored.
        Assert.All(fleet, e => Assert.Equal(0.25, e.FailureRate, 3));
        Assert.Equal(7, detail.Seed);
    }

    // ---- T2: Stored fleet on create (default) ----

    [Fact]
    public async Task P1_StoredFleetDefault_DefaultFleetPersistedWhenNoConfig()
    {
        using var factory = new P1ApiFactory();
        var client = factory.CreateClient();

        var created = await CreateScheduleAsync(client, "def-1");
        Assert.Equal(HttpStatusCode.Created, created);

        var detail = await GetDetailAsync(client, "def-1");
        Assert.NotNull(detail);

        // Default fleet: ep-1 0.1, ep-2 0.0, ep-3 0.3, seed 42.
        var expected = new (string Id, double FailureRate)[]
        {
            ("ep-1", 0.1),
            ("ep-2", 0.0),
            ("ep-3", 0.3),
        };
        Assert.Equal(expected.Length, detail!.Fleet.Count);
        for (var i = 0; i < expected.Length; i++)
        {
            Assert.Equal(expected[i].Id, detail.Fleet[i].Id);
            Assert.Equal(expected[i].FailureRate, detail.Fleet[i].FailureRate, 3);
        }
        Assert.Equal(42, detail.Seed);
    }

    // ---- T3: List endpoint exists ----

    [Fact]
    public async Task P1_ListEndpoint_Returns200AndJsonArray()
    {
        using var factory = new P1ApiFactory();
        var client = factory.CreateClient();

        await CreateScheduleAsync(client, "list-1");

        var resp = await client.GetAsync("/api/schedules");
        Assert.Equal(HttpStatusCode.OK, resp.StatusCode);
        Assert.Equal("application/json", resp.Content.Headers.ContentType?.MediaType);

        using var doc = JsonDocument.Parse(await resp.Content.ReadAsStringAsync());
        Assert.Equal(JsonValueKind.Array, doc.RootElement.ValueKind);
    }

    // ---- T4: List ordering - newest first ----

    [Fact]
    public async Task P1_ListOrdering_SchedulesReturnedNewestFirst()
    {
        using var factory = new P1ApiFactory();
        var client = factory.CreateClient();

        // Create in a known order; a small delay keeps creation timestamps distinct.
        await CreateScheduleAsync(client, "first");
        await Task.Delay(30);
        await CreateScheduleAsync(client, "second");
        await Task.Delay(30);
        await CreateScheduleAsync(client, "third");

        var list = await GetListAsync(client);

        // Newest-first by creation time => "third", "second", "first".
        Assert.Equal(new[] { "third", "second", "first" }, list.Select(x => x.Id));

        // Creation times must be monotonically non-increasing.
        for (var i = 1; i < list.Count; i++)
        {
            Assert.True(list[i - 1].Created >= list[i].Created,
                "List must be ordered newest-first by created timestamp.");
        }
    }

    // ---- T5: List includes summary fields ----

    [Fact]
    public async Task P1_ListSummaryFields_EachItemHasIdStatusAndCreated()
    {
        using var factory = new P1ApiFactory();
        var client = factory.CreateClient();

        await CreateScheduleAsync(client, "sum-1");
        await CreateScheduleAsync(client, "sum-2");

        var list = await GetListAsync(client);

        Assert.NotEmpty(list);
        foreach (var item in list)
        {
            Assert.False(string.IsNullOrWhiteSpace(item.Id), "List item must expose 'id'.");
            Assert.False(string.IsNullOrWhiteSpace(item.Status), "List item must expose 'status'.");
            Assert.NotEqual(default, item.Created); // 'created' timestamp present.
        }
    }

    // ---- T6: Empty list ----

    [Fact]
    public async Task P1_EmptyList_Returns200WithEmptyArray()
    {
        using var factory = new P1ApiFactory();
        var client = factory.CreateClient();

        var resp = await client.GetAsync("/api/schedules");
        Assert.Equal(HttpStatusCode.OK, resp.StatusCode);

        using var doc = JsonDocument.Parse(await resp.Content.ReadAsStringAsync());
        Assert.Equal(JsonValueKind.Array, doc.RootElement.ValueKind);
        Assert.Equal(0, doc.RootElement.GetArrayLength());
    }

    // ---- T7: Regression - existing behavior preserved ----

    [Fact]
    public async Task P1_Regression_ExistingEndpointsStillWork()
    {
        using var factory = new P1ApiFactory();
        var client = factory.CreateClient();

        // Health still works.
        var health = await client.GetAsync("/api/health");
        Assert.Equal(HttpStatusCode.OK, health.StatusCode);

        // POST /api/schedules still returns 201 Created.
        var created = await CreateScheduleAsync(client, "reg-1");
        Assert.Equal(HttpStatusCode.Created, created);

        // The live EngineSession starts: status reports "running", and a control
        // operation mutates it (proving a live session exists, not just a record).
        var status = await GetStatusAsync(client, "reg-1");
        Assert.Equal("running", status);

        var pause = await client.PostAsync("/api/schedules/reg-1/pause", null);
        Assert.Equal(HttpStatusCode.OK, pause.StatusCode);

        // GET /api/schedules/{id}/status still works after the mutation.
        var statusAfter = await GetStatusAsync(client, "reg-1");
        Assert.Equal("paused", statusAfter);
    }

    // ---- Helpers ----

    private static async Task<HttpStatusCode> CreateScheduleAsync(
        HttpClient client, string id, int? fleetSize = null, double? failureRate = null, int? seed = null)
    {
        var resp = await client.PostAsJsonAsync("/api/schedules", new
        {
            id,
            package = "pkg",
            groupId = "grp",
            fleetSize,
            failureRate,
            seed,
        });
        return resp.StatusCode;
    }

    private static async Task<string?> GetStatusAsync(HttpClient client, string id)
    {
        var resp = await client.GetAsync($"/api/schedules/{id}/status");
        resp.EnsureSuccessStatusCode();
        using var doc = JsonDocument.Parse(await resp.Content.ReadAsStringAsync());
        return doc.RootElement.GetProperty("status").GetString();
    }

    /// <summary>
    /// Fetches the P1 detail contract <c>GET /api/schedules/{id}</c>, which exposes
    /// the persisted fleet (id + failureRate per endpoint) and seed. This is the
    /// contract the dashboard (P3) will later rely on to load a schedule's fleet.
    /// </summary>
    private static async Task<ScheduleDetail?> GetDetailAsync(HttpClient client, string id)
    {
        var resp = await client.GetAsync($"/api/schedules/{id}");
        Assert.Equal(HttpStatusCode.OK, resp.StatusCode);

        using var doc = JsonDocument.Parse(await resp.Content.ReadAsStringAsync());
        var root = doc.RootElement;

        var detail = new ScheduleDetail { Seed = root.GetProperty("seed").GetInt32() };
        var fleet = root.GetProperty("fleet");
        foreach (var e in fleet.EnumerateArray())
        {
            detail.Fleet.Add(new FleetEndpoint(
                e.GetProperty("id").GetString()!,
                e.GetProperty("failureRate").GetDouble()));
        }
        return detail;
    }

    private static async Task<List<ScheduleSummary>> GetListAsync(HttpClient client)
    {
        var resp = await client.GetAsync("/api/schedules");
        resp.EnsureSuccessStatusCode();

        using var doc = JsonDocument.Parse(await resp.Content.ReadAsStringAsync());
        var list = new List<ScheduleSummary>();
        foreach (var e in doc.RootElement.EnumerateArray())
        {
            list.Add(new ScheduleSummary
            {
                Id = e.GetProperty("id").GetString()!,
                Status = e.GetProperty("status").GetString()!,
                Created = e.GetProperty("created").GetDateTimeOffset(),
            });
        }
        return list;
    }

    private sealed class ScheduleDetail
    {
        public List<FleetEndpoint> Fleet { get; } = new();
        public int Seed { get; set; }
    }

    private sealed record FleetEndpoint(string Id, double FailureRate);

    private sealed class ScheduleSummary
    {
        public string Id { get; set; } = "";
        public string Status { get; set; } = "";
        public DateTimeOffset Created { get; set; }
    }
}

/// <summary>
/// Bootstraps the real API host (via <c>Program</c>) and points the engine
/// bridge at the repo's <c>python/</c> directory regardless of the test
/// runner's working directory.
/// </summary>
public class P1ApiFactory : WebApplicationFactory<Program>
{
    public P1ApiFactory()
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
