using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Tests for the server-side <see cref="EngineSession"/> (Sprint 12 / B2): a
/// holder for one live <c>Rollout</c> that applies pause/resume/rollback/tick
/// in place against the B1 persistent bridge, deterministically.
/// </summary>
public class EngineSessionTests
{
    private static readonly EngineRequest SampleRequest = new(
        new List<EngineEndpointRequest>
        {
            new("ep-1", 0.1),
            new("ep-2", 0.0),
        },
        Seed: 42);

    public EngineSessionTests()
    {
        // Point the bridge at the repo's python/ directory regardless of the
        // test runner's working directory.
        Environment.SetEnvironmentVariable("PATCHORCH_PYTHON_DIR", FindPythonDir());
    }

    // ---- T1: Session holds a live rollout ----

    [Fact]
    public void B2_HoldsRollout_SessionHoldsLiveRolloutWithMatchingConfig()
    {
        using var session = new EngineSession(new EngineBridge(), SampleRequest);

        // The session exposes a single live rollout state.
        var state = session.State;
        Assert.NotNull(state);
        Assert.Equal(SampleRequest.Endpoints.Count, state.Endpoints.Count);

        // Endpoints and seed match the configuration.
        for (var i = 0; i < SampleRequest.Endpoints.Count; i++)
        {
            Assert.Equal(SampleRequest.Endpoints[i].Id, state.Endpoints[i].Id);
        }
        Assert.Equal(SampleRequest.Seed, session.Request.Seed);

        // The live rollout is running after start.
        Assert.All(state.Endpoints, e => Assert.Equal("running", e.State));
    }

    // ---- T2: Operations mutate in place ----

    [Fact]
    public void B2_MutateInPlace_PauseResumeRollbackTickMutateSameLiveRollout()
    {
        using var session = new EngineSession(new EngineBridge(), SampleRequest);

        // Pause: running -> paused.
        var paused = session.Pause();
        Assert.All(paused.Endpoints, e => Assert.Equal("paused", e.State));
        Assert.Same(paused, session.State);

        // Tick while paused must NOT advance (progress frozen).
        var frozen = session.Tick(5);
        Assert.All(frozen.Endpoints, e => Assert.Equal(0.0, e.Progress));

        // Resume: paused -> running.
        var resumed = session.Resume();
        Assert.All(resumed.Endpoints, e => Assert.Equal("running", e.State));
        Assert.Same(resumed, session.State);

        // Tick advances the live rollout deterministically.
        var ticked = session.Tick(1);
        Assert.Contains(ticked.Endpoints, e => e.Progress > 0.0);
        Assert.Same(ticked, session.State);

        // Rollback: running -> rolled_back (succeeded stay succeeded).
        var rolledBack = session.Rollback();
        Assert.True(rolledBack.RolledBack);
        Assert.All(rolledBack.Endpoints, e => Assert.Equal("rolled_back", e.State));
        Assert.Same(rolledBack, session.State);

        // The same in-place instance is exposed throughout.
        Assert.Same(session.State, rolledBack);
    }

    // ---- T3: Deterministic ----

    [Fact]
    public void B2_Deterministic_SameConfigAndSeedProduceSameResult()
    {
        using var a = new EngineSession(new EngineBridge(), SampleRequest);
        using var b = new EngineSession(new EngineBridge(), SampleRequest);

        // Advance both identically.
        a.Tick(3);
        b.Tick(3);

        var sa = a.State;
        var sb = b.State;
        Assert.Equal(sa.Endpoints.Count, sb.Endpoints.Count);
        for (var i = 0; i < sa.Endpoints.Count; i++)
        {
            Assert.Equal(sa.Endpoints[i].Id, sb.Endpoints[i].Id);
            Assert.Equal(sa.Endpoints[i].State, sb.Endpoints[i].State);
            Assert.Equal(sa.Endpoints[i].Progress, sb.Endpoints[i].Progress);
        }
    }

    [Fact]
    public void B2_Deterministic_TickSequenceReproducibleForFixedSeed()
    {
        using var a = new EngineSession(new EngineBridge(), SampleRequest);
        using var b = new EngineSession(new EngineBridge(), SampleRequest);

        var seqA = new List<double>();
        var seqB = new List<double>();
        for (var i = 0; i < 5; i++)
        {
            seqA.Add(a.Tick(1).Endpoints[0].Progress);
            seqB.Add(b.Tick(1).Endpoints[0].Progress);
        }

        Assert.Equal(seqA, seqB);
    }

    // ---- T4: State transitions valid ----

    [Fact]
    public void B2_Transitions_InvalidTransitionsHandledGracefully()
    {
        using var session = new EngineSession(new EngineBridge(), SampleRequest);

        // Pause when already paused: no crash, state stays paused.
        session.Pause();
        var doublePause = session.Pause();
        Assert.All(doublePause.Endpoints, e => Assert.Equal("paused", e.State));

        // Rollback after succeeded: succeeded stay succeeded, others rolled_back.
        // Use a config where ep-1 always fails and ep-2 always succeeds.
        using var mixed = new EngineSession(new EngineBridge(), new EngineRequest(
            new List<EngineEndpointRequest>
            {
                new("ep-1", 1.0),
                new("ep-2", 0.0),
            },
            Seed: 7));
        mixed.Tick(100); // ep-1 fails on first tick, ep-2 reaches succeeded
        var rolledBack = mixed.Rollback();
        Assert.True(rolledBack.RolledBack);
        Assert.Contains(rolledBack.Endpoints, e => e.Id == "ep-2" && e.State == "succeeded");
        Assert.Contains(rolledBack.Endpoints, e => e.Id == "ep-1" && e.State == "rolled_back");

        // Session reports a consistent state after each operation.
        session.Rollback();
        var final = session.Refresh();
        Assert.True(final.RolledBack);
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
