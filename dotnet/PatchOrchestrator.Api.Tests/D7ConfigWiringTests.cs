using System.Net.Http.Json;
using Microsoft.AspNetCore.Mvc.Testing;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Sprint 31 (D7) tests: passing the configured fleet size, failure rate, and
/// seed into the live <see cref="EngineSession"/> (B2) when starting a rollout.
/// The request is built from config via <see cref="EngineRequestFactory"/> so a
/// rollout started from configured D1–D3 values uses those values (endpoints
/// count = fleet size, each endpoint's failure rate, and the seed).
/// </summary>
public class D7ConfigWiringTests
{
    public D7ConfigWiringTests()
    {
        // Point the bridge at the repo's python/ directory regardless of the
        // test runner's working directory.
        Environment.SetEnvironmentVariable("PATCHORCH_PYTHON_DIR", FindPythonDir());
    }

    // ---- T1: Config reaches engine ----

    [Fact]
    public void D7_ConfigReachesEngine_RequestBuiltFromConfigPassesValuesToSession()
    {
        const int fleetSize = 5;
        const double failureRate = 0.4;
        const int seed = 1234;

        var request = EngineRequestFactory.Build(fleetSize, failureRate, seed);
        using var session = new EngineSession(new EngineBridge(), request);

        // The session's request carries the configured fleet size and seed.
        Assert.Equal(fleetSize, session.Request.Endpoints.Count);
        Assert.Equal(seed, session.Request.Seed);

        // Endpoints count matches fleet size and each endpoint uses the
        // configured failure rate.
        Assert.Equal(fleetSize, session.State.Endpoints.Count);
        Assert.All(session.Request.Endpoints, e => Assert.Equal(failureRate, e.FailureRate));
        Assert.All(session.Request.Endpoints.Select(e => e.Id),
            id => Assert.StartsWith("ep-", id));

        // A live rollout is running after start.
        Assert.All(session.State.Endpoints, e => Assert.Equal("running", e.State));
    }

    // ---- T2: Rollout uses configured values ----

    [Fact]
    public void D7_RolloutUsesConfig_RolloutMatchesConfigAndIsDeterministicForSeed()
    {
        const int fleetSize = 4;
        const double failureRate = 0.25;
        const int seed = 999;

        var request = EngineRequestFactory.Build(fleetSize, failureRate, seed);
        using var session = new EngineSession(new EngineBridge(), request);

        // The rollout has the configured number of endpoints.
        Assert.Equal(fleetSize, session.State.Endpoints.Count);

        // Endpoint ids are ep-1..ep-N and use the configured failure rate.
        var expectedIds = Enumerable.Range(1, fleetSize).Select(i => $"ep-{i}").ToList();
        Assert.Equal(expectedIds, session.State.Endpoints.Select(e => e.Id).ToList());
        Assert.All(session.Request.Endpoints, e => Assert.Equal(failureRate, e.FailureRate));

        // Advance the rollout; it must be deterministic for the configured seed.
        session.Tick(4);
        var first = session.State;

        using var again = new EngineSession(new EngineBridge(), request);
        again.Tick(4);
        var second = again.State;

        Assert.Equal(first.Endpoints.Count, second.Endpoints.Count);
        for (var i = 0; i < first.Endpoints.Count; i++)
        {
            Assert.Equal(first.Endpoints[i].Id, second.Endpoints[i].Id);
            Assert.Equal(first.Endpoints[i].State, second.Endpoints[i].State);
            Assert.Equal(first.Endpoints[i].Progress, second.Endpoints[i].Progress);
        }
    }

    // ---- T3: Regression - live session still works; guards intact ----

    [Fact]
    public void D7_Regression_LiveSessionStillWorksAndInvalidConfigRejected()
    {
        // A small configured fleet still drives a working live session.
        var request = EngineRequestFactory.Build(3, 0.1, 42);
        using var session = new EngineSession(new EngineBridge(), request);

        Assert.Equal(3, session.State.Endpoints.Count);
        Assert.Equal(42, session.Request.Seed);

        var paused = session.Pause();
        Assert.All(paused.Endpoints, e => Assert.Equal("paused", e.State));
        var resumed = session.Resume();
        Assert.All(resumed.Endpoints, e => Assert.Equal("running", e.State));

        // Invalid config is rejected before any request is built.
        Assert.Throws<ArgumentOutOfRangeException>(() => EngineRequestFactory.Build(0, 0.5, 1));
        Assert.Throws<ArgumentOutOfRangeException>(() => EngineRequestFactory.Build(3, -0.1, 1));
        Assert.Throws<ArgumentOutOfRangeException>(() => EngineRequestFactory.Build(3, 1.5, 1));
        Assert.Throws<ArgumentOutOfRangeException>(() => EngineRequestFactory.Build(3, double.NaN, 1));
    }

    // ---- T3b: API wiring regression - a configured schedule reaches the session ----

    [Fact]
    public async Task D7_Regression_ApiScheduleUsesConfiguredValues()
    {
        using var factory = new D7ApiFactory();
        var client = factory.CreateClient();

        // Create a schedule with configured fleet size, failure rate, and seed.
        var create = await client.PostAsJsonAsync("/api/schedules", new
        {
            id = "d7-config",
            package = "pkg",
            groupId = "grp",
            fleetSize = 6,
            failureRate = 0.2,
            seed = 77,
        });
        create.EnsureSuccessStatusCode();

        // Tick the live session and confirm the configured fleet of 6 endpoints
        // is driving the rollout (all six endpoints present).
        var tick = await client.PostAsync("/api/schedules/d7-config/tick", null);
        tick.EnsureSuccessStatusCode();
        var body = await tick.Content.ReadAsStringAsync();
        using var doc = System.Text.Json.JsonDocument.Parse(body);
        var endpoints = doc.RootElement.GetProperty("endpoints");
        Assert.Equal(6, endpoints.GetArrayLength());
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
/// Bootstraps the real API host (via <c>Program</c>) and points the engine
/// bridge at the repo's <c>python/</c> directory regardless of the test
/// runner's working directory.
/// </summary>
public class D7ApiFactory : WebApplicationFactory<Program>
{
    public D7ApiFactory()
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
