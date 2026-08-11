using System.Diagnostics;
using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Tests for the persistent <see cref="EngineBridge"/> (Sprint 11 / B1): a
/// single long-lived Python subprocess reused across calls, live state
/// persistence, clean shutdown, and no regression to single-call behavior.
/// </summary>
public class EngineBridgePersistentTests
{
    private static readonly EngineRequest SampleRequest = new(
        new List<EngineEndpointRequest>
        {
            new("ep-1", 0.1),
            new("ep-2", 0.0),
        },
        Seed: 42);

    public EngineBridgePersistentTests()
    {
        // Point the bridge at the repo's python/ directory regardless of the
        // test runner's working directory.
        Environment.SetEnvironmentVariable("PATCHORCH_PYTHON_DIR", FindPythonDir());
    }

    // ---- T1: Subprocess lifecycle (single long-lived process) ----

    [Fact]
    public void B1_SubprocessLifecycle_ReusesOneProcessAcrossCalls()
    {
        using var bridge = new EngineBridge();

        Assert.False(bridge.IsProcessRunning, "Subprocess should start lazily.");

        bridge.Run(SampleRequest);
        Assert.True(bridge.IsProcessRunning, "Subprocess should be alive after first call.");
        var firstPid = bridge.ProcessId;
        Assert.Equal(1, bridge.ProcessStartCount);

        bridge.Run(SampleRequest);
        bridge.Run(SampleRequest);
        bridge.GetState();

        Assert.Equal(1, bridge.ProcessStartCount);
        Assert.Equal(firstPid, bridge.ProcessId);
        Assert.True(bridge.IsProcessRunning);
    }

    // ---- T2: State persists between calls ----

    [Fact]
    public void B1_StatePersistence_LiveStateMutatesAcrossCalls()
    {
        using var bridge = new EngineBridge();

        // Start a rollout: pending -> running.
        var started = bridge.Start(SampleRequest);
        Assert.All(started.Endpoints, e => Assert.Equal("running", e.State));

        // Pause freezes progress: tick while paused must NOT advance.
        bridge.Pause();
        var frozen = bridge.Tick(5);
        Assert.All(frozen.Endpoints, e => Assert.Equal(0.0, e.Progress));

        // Resume allows advancement: tick advances running endpoints.
        bridge.Resume();
        var ticked = bridge.Tick(1);
        Assert.Contains(ticked.Endpoints, e => e.Progress > 0.0);

        // Rollback: running -> rolled_back (succeeded stay succeeded).
        var rolledBack = bridge.Rollback();
        Assert.True(rolledBack.RolledBack);
        Assert.All(rolledBack.Endpoints, e => Assert.Equal("rolled_back", e.State));

        // State persists: a fresh GetState sees the same rolled-back state.
        var state = bridge.GetState();
        Assert.True(state.RolledBack);
        Assert.All(state.Endpoints, e => Assert.Equal("rolled_back", e.State));
    }

    // ---- T3: Clean shutdown ----

    [Fact]
    public void B1_CleanShutdown_DisposeTerminatesSubprocessAndIsIdempotent()
    {
        var bridge = new EngineBridge();
        bridge.Run(SampleRequest);
        Assert.True(bridge.IsProcessRunning);
        var pid = bridge.ProcessId;

        bridge.Dispose();

        Assert.False(bridge.IsProcessRunning);
        Assert.Equal(-1, bridge.ProcessId);
        AssertProcessGone(pid);

        // Idempotent: disposing again does not error.
        bridge.Dispose();
    }

    // ---- T4: Existing single-call behavior preserved (regression) ----

    [Fact]
    public void B1_Regression_SingleRunIsDeterministicForFixedSeed()
    {
        using var bridge = new EngineBridge();

        var first = bridge.Run(SampleRequest);
        var second = bridge.Run(SampleRequest);

        Assert.Equal(first.RolledBack, second.RolledBack);
        Assert.Equal(first.Endpoints.Count, second.Endpoints.Count);
        for (var i = 0; i < first.Endpoints.Count; i++)
        {
            Assert.Equal(first.Endpoints[i].Id, second.Endpoints[i].Id);
            Assert.Equal(first.Endpoints[i].State, second.Endpoints[i].State);
            Assert.Equal(first.Endpoints[i].Progress, second.Endpoints[i].Progress);
        }
    }

    private static void AssertProcessGone(int pid)
    {
        if (pid <= 0)
        {
            return;
        }
        try
        {
            using var proc = Process.GetProcessById(pid);
            Assert.True(proc.HasExited, $"Python subprocess (pid {pid}) was not terminated.");
        }
        catch (ArgumentException)
        {
            // Process no longer exists — exactly what we want.
        }
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
