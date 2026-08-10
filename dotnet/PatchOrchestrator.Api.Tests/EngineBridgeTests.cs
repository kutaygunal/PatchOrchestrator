using PatchOrchestrator.Api;
using Xunit;

namespace PatchOrchestrator.Api.Tests;

/// <summary>
/// Unit/integration tests for the API &lt;-&gt; engine bridge. These exercise the
/// real <see cref="EngineBridge"/>, which spawns the Python engine as a
/// subprocess, so they verify the full bridge contract end-to-end.
/// </summary>
public class EngineBridgeTests
{
    private static readonly EngineRequest SampleRequest = new(
        new List<EngineEndpointRequest>
        {
            new("ep-1", 0.1),
            new("ep-2", 0.0),
        },
        Seed: 42);

    public EngineBridgeTests()
    {
        // Point the bridge at the repo's python/ directory regardless of the
        // test runner's working directory.
        Environment.SetEnvironmentVariable("PATCHORCH_PYTHON_DIR", FindPythonDir());
    }

    [Fact]
    public void Run_ReturnsParsedResults()
    {
        var bridge = new EngineBridge();

        var result = bridge.Run(SampleRequest);

        Assert.NotNull(result);
        Assert.Equal(2, result.Endpoints.Count);
        Assert.Contains(result.Endpoints, e => e.Id == "ep-1");
        Assert.Contains(result.Endpoints, e => e.Id == "ep-2");
        Assert.All(result.Endpoints, e =>
        {
            Assert.False(string.IsNullOrWhiteSpace(e.State));
            Assert.InRange(e.Progress, 0.0, 100.0);
        });
    }

    [Fact]
    public void Run_IsDeterministicForFixedSeed()
    {
        var bridge = new EngineBridge();

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

    [Fact]
    public void Run_ZeroFailureRateEndpointSucceeds()
    {
        var bridge = new EngineBridge();

        var result = bridge.Run(SampleRequest);

        var ep2 = Assert.Single(result.Endpoints, e => e.Id == "ep-2");
        Assert.Equal("succeeded", ep2.State);
        Assert.Equal(100.0, ep2.Progress);
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
