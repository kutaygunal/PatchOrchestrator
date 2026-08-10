using System.Diagnostics;
using System.Text.Json;

namespace PatchOrchestrator.Api;

/// <summary>Request passed to the Python simulation engine.</summary>
public record EngineEndpointRequest(string Id, double FailureRate);

/// <summary>Request passed to the Python simulation engine.</summary>
public record EngineRequest(IReadOnlyList<EngineEndpointRequest> Endpoints, int Seed);

/// <summary>One endpoint's result from the engine.</summary>
public record EngineEndpointResult(string Id, string State, double Progress);

/// <summary>Parsed result returned by the engine.</summary>
public record EngineResult(IReadOnlyList<EngineEndpointResult> Endpoints, bool RolledBack);

/// <summary>Drives the Python simulation engine over a JSON subprocess interface.</summary>
public interface IEngineBridge
{
    EngineResult Run(EngineRequest request);
}

/// <summary>
/// Invokes <c>python bridge.py</c> via a subprocess, passes the request JSON on
/// stdin, and deserializes the stdout JSON. The <c>python/</c> directory is
/// resolved via the <c>PATCHORCH_PYTHON_DIR</c> env var (default: <c>../python</c>
/// relative to the API project).
/// </summary>
public class EngineBridge : IEngineBridge
{
    private readonly string _pythonDir;
    private readonly string _pythonExe;

    public EngineBridge()
    {
        _pythonDir = ResolvePythonDir();
        _pythonExe = "python"; // 3.11, NOT python3 (Windows Store alias).
    }

    public EngineResult Run(EngineRequest request)
    {
        var requestJson = JsonSerializer.Serialize(new
        {
            endpoints = request.Endpoints.Select(e => new { id = e.Id, failure_rate = e.FailureRate }),
            seed = request.Seed,
        });

        var psi = new ProcessStartInfo
        {
            FileName = _pythonExe,
            WorkingDirectory = _pythonDir,
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true,
        };
        psi.ArgumentList.Add("bridge.py");
        psi.Environment["PYTHONPATH"] = _pythonDir;

        using var process = Process.Start(psi)
            ?? throw new InvalidOperationException("Failed to start python bridge process.");

        process.StandardInput.Write(requestJson);
        process.StandardInput.Close();
        var stdout = process.StandardOutput.ReadToEnd();
        var stderr = process.StandardError.ReadToEnd();
        process.WaitForExit();

        if (process.ExitCode != 0)
        {
            throw new InvalidOperationException(
                $"Python bridge failed (exit {process.ExitCode}): {stderr}");
        }

        var options = new JsonSerializerOptions
        {
            PropertyNamingPolicy = JsonNamingPolicy.SnakeCaseLower,
            PropertyNameCaseInsensitive = true,
        };
        var result = JsonSerializer.Deserialize<EngineResult>(stdout, options)
            ?? throw new InvalidOperationException("Python bridge returned an empty result.");

        return result;
    }

    private static string ResolvePythonDir()
    {
        var env = Environment.GetEnvironmentVariable("PATCHORCH_PYTHON_DIR");
        if (!string.IsNullOrWhiteSpace(env))
        {
            return Path.GetFullPath(env);
        }

        // Default: ../python relative to the API project. Walk up from the
        // assembly location to find a directory containing python/.
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

        return Path.GetFullPath("../python");
    }
}
