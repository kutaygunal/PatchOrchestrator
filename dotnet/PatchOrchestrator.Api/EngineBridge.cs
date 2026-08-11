using System.Diagnostics;
using System.Text.Json;
using Microsoft.Extensions.Logging;

namespace PatchOrchestrator.Api;

/// <summary>Request passed to the Python simulation engine.</summary>
public record EngineEndpointRequest(string Id, double FailureRate);

/// <summary>Request passed to the Python simulation engine.</summary>
public record EngineRequest(IReadOnlyList<EngineEndpointRequest> Endpoints, int Seed);

/// <summary>One endpoint's result from the engine.</summary>
public record EngineEndpointResult(string Id, string State, double Progress);

/// <summary>Parsed result returned by the engine.</summary>
public record EngineResult(IReadOnlyList<EngineEndpointResult> Endpoints, bool RolledBack);

/// <summary>Drives the Python simulation engine over a persistent JSON subprocess interface.</summary>
public interface IEngineBridge : IDisposable
{
    /// <summary>Run a rollout to completion and return the final state.</summary>
    EngineResult Run(EngineRequest request);

    /// <summary>Start a rollout (pending -&gt; running) and return the live state.</summary>
    EngineResult Start(EngineRequest request);

    /// <summary>Pause the live rollout (running -&gt; paused).</summary>
    EngineResult Pause();

    /// <summary>Resume the live rollout (paused -&gt; running).</summary>
    EngineResult Resume();

    /// <summary>Roll back the live rollout (running/paused/failed -&gt; rolled_back).</summary>
    EngineResult Rollback();

    /// <summary>Advance the live rollout by the given number of steps.</summary>
    EngineResult Tick(int steps = 1);

    /// <summary>Return the current live rollout state without mutating it.</summary>
    EngineResult GetState();
}

/// <summary>
/// Invokes <c>python bridge_persistent.py</c> via a single long-lived
/// subprocess, sends one JSON request per line on stdin, and reads one JSON
/// response per line from stdout. The subprocess is started lazily on the
/// first call and reused across all subsequent calls so live engine state
/// (pause/resume/rollback/tick) persists between calls. The <c>python/</c>
/// directory is resolved via the <c>PATCHORCH_PYTHON_DIR</c> env var
/// (default: <c>../python</c> relative to the API project).
/// </summary>
public class EngineBridge : IEngineBridge
{
    private readonly object _lock = new();
    private readonly string _pythonDir;
    private readonly string _pythonExe;
    private readonly ILogger<EngineBridge>? _logger;

    private Process? _process;
    private StreamWriter? _stdin;
    private StreamReader? _stdout;
    private Task? _stderrDrain;
    private bool _disposed;
    private int _processStartCount;

    // The logger is optional so the parameterless constructor remains usable
    // in tests; DI supplies the real logger at runtime.
    public EngineBridge(ILogger<EngineBridge>? logger = null)
    {
        _pythonDir = ResolvePythonDir();
        _pythonExe = "python"; // 3.11, NOT python3 (Windows Store alias).
        _logger = logger;
    }

    /// <summary>Number of Python subprocesses started (for lifecycle tests).</summary>
    public int ProcessStartCount => _processStartCount;

    /// <summary>Whether the long-lived Python subprocess is currently alive.</summary>
    public bool IsProcessRunning => _process != null && !_process.HasExited;

    /// <summary>PID of the current Python subprocess, or -1 if none is running.</summary>
    public int ProcessId => _process != null && !_process.HasExited ? _process.Id : -1;

    public EngineResult Run(EngineRequest request)
    {
        lock (_lock)
        {
            EnsureStarted();
            _logger?.LogInformation(
                "EngineBridge: running Python engine with {Count} endpoint(s), seed {Seed}",
                request.Endpoints.Count, request.Seed);
            return SendCommand(new Dictionary<string, object?>
            {
                ["cmd"] = "run",
                ["endpoints"] = request.Endpoints
                    .Select(e => new { id = e.Id, failure_rate = e.FailureRate })
                    .ToArray(),
                ["seed"] = request.Seed,
            });
        }
    }

    public EngineResult Start(EngineRequest request)
    {
        lock (_lock)
        {
            EnsureStarted();
            return SendCommand(new Dictionary<string, object?>
            {
                ["cmd"] = "start",
                ["endpoints"] = request.Endpoints
                    .Select(e => new { id = e.Id, failure_rate = e.FailureRate })
                    .ToArray(),
                ["seed"] = request.Seed,
            });
        }
    }

    public EngineResult Pause()
    {
        lock (_lock)
        {
            EnsureStarted();
            return SendCommand(new Dictionary<string, object?> { ["cmd"] = "pause" });
        }
    }

    public EngineResult Resume()
    {
        lock (_lock)
        {
            EnsureStarted();
            return SendCommand(new Dictionary<string, object?> { ["cmd"] = "resume" });
        }
    }

    public EngineResult Rollback()
    {
        lock (_lock)
        {
            EnsureStarted();
            return SendCommand(new Dictionary<string, object?> { ["cmd"] = "rollback" });
        }
    }

    public EngineResult Tick(int steps = 1)
    {
        lock (_lock)
        {
            EnsureStarted();
            return SendCommand(new Dictionary<string, object?>
            {
                ["cmd"] = "tick",
                ["steps"] = steps,
            });
        }
    }

    public EngineResult GetState()
    {
        lock (_lock)
        {
            EnsureStarted();
            return SendCommand(new Dictionary<string, object?> { ["cmd"] = "state" });
        }
    }

    public void Dispose()
    {
        lock (_lock)
        {
            if (_disposed)
            {
                return;
            }
            _disposed = true;
            Shutdown();
        }
    }

    private void EnsureStarted()
    {
        if (_process != null && !_process.HasExited)
        {
            return;
        }

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
        psi.ArgumentList.Add("bridge_persistent.py");
        psi.Environment["PYTHONPATH"] = _pythonDir;

        _process = Process.Start(psi)
            ?? throw new InvalidOperationException("Failed to start python bridge process.");
        _processStartCount++;
        _stdin = _process.StandardInput;
        _stdout = _process.StandardOutput;
        _stderrDrain = Task.Run(() => DrainStderr(_process.StandardError));

        _logger?.LogInformation("EngineBridge: started persistent Python bridge (pid {Pid})", _process.Id);
    }

    private void DrainStderr(StreamReader reader)
    {
        try
        {
            string? line;
            while ((line = reader.ReadLine()) != null)
            {
                _logger?.LogInformation("EngineBridge[stderr]: {Line}", line);
            }
        }
        catch
        {
            // Process exited; nothing more to drain.
        }
    }

    private EngineResult SendCommand(Dictionary<string, object?> payload)
    {
        var json = JsonSerializer.Serialize(payload);
        _stdin!.WriteLine(json);
        _stdin.Flush();

        var line = _stdout!.ReadLine();
        if (line == null)
        {
            throw new InvalidOperationException("Python bridge closed unexpectedly.");
        }

        using var doc = JsonDocument.Parse(line);
        var root = doc.RootElement;
        if (root.TryGetProperty("ok", out var ok) && ok.GetBoolean() == false)
        {
            var error = root.TryGetProperty("error", out var err)
                ? err.GetString()
                : "unknown error";
            throw new InvalidOperationException($"Python bridge error: {error}");
        }

        return ParseResult(root);
    }

    private static EngineResult ParseResult(JsonElement root)
    {
        var endpoints = new List<EngineEndpointResult>();
        if (root.TryGetProperty("endpoints", out var endpointsEl) && endpointsEl.ValueKind == JsonValueKind.Array)
        {
            foreach (var ep in endpointsEl.EnumerateArray())
            {
                endpoints.Add(new EngineEndpointResult(
                    ep.GetProperty("id").GetString() ?? string.Empty,
                    ep.GetProperty("state").GetString() ?? string.Empty,
                    ep.GetProperty("progress").GetDouble()));
            }
        }

        var rolledBack = root.TryGetProperty("rolled_back", out var rb) && rb.GetBoolean();
        return new EngineResult(endpoints, rolledBack);
    }

    private void Shutdown()
    {
        if (_process == null || _process.HasExited)
        {
            return;
        }

        try
        {
            _stdin?.WriteLine(JsonSerializer.Serialize(new { cmd = "shutdown" }));
            _stdin?.Flush();
            if (!_process.WaitForExit(3000))
            {
                _logger?.LogWarning("EngineBridge: Python bridge did not exit cleanly; killing it.");
                _process.Kill(entireProcessTree: true);
            }
        }
        catch
        {
            // Process already gone.
        }
        finally
        {
            _process?.Dispose();
            _process = null;
            _stdin = null;
            _stdout = null;
        }
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
