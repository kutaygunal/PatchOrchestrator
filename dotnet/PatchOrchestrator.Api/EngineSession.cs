namespace PatchOrchestrator.Api;

/// <summary>
/// Server-side holder for one live <c>Rollout</c> instance (Sprint 12 / B2).
/// Wraps the B1 persistent <see cref="IEngineBridge"/> so that a single
/// long-lived Python subprocess keeps one live rollout whose state is mutated
/// in place by pause/resume/rollback/tick. The session is deterministic
/// (seeded via the request) and exposes the current rollout state so it can be
/// read back.
/// </summary>
public class EngineSession : IDisposable
{
    private readonly IEngineBridge _bridge;
    private readonly EngineRequest _request;
    private EngineResult _state;

    /// <summary>
    /// Creates a session and starts a live rollout from the given request
    /// (pending -&gt; running) via the persistent bridge.
    /// </summary>
    public EngineSession(IEngineBridge bridge, EngineRequest request)
    {
        _bridge = bridge ?? throw new ArgumentNullException(nameof(bridge));
        _request = request ?? throw new ArgumentNullException(nameof(request));
        _state = _bridge.Start(request);
    }

    /// <summary>The configuration this session was started with.</summary>
    public EngineRequest Request => _request;

    /// <summary>The current live rollout state (read back from the engine).</summary>
    public EngineResult State => _state;

    /// <summary>Pause the live rollout (running -&gt; paused), mutating it in place.</summary>
    public EngineResult Pause() => _state = _bridge.Pause();

    /// <summary>Resume the live rollout (paused -&gt; running), mutating it in place.</summary>
    public EngineResult Resume() => _state = _bridge.Resume();

    /// <summary>Roll back the live rollout (running/paused/failed -&gt; rolled_back).</summary>
    public EngineResult Rollback() => _state = _bridge.Rollback();

    /// <summary>Advance the live rollout deterministically by the given number of steps.</summary>
    public EngineResult Tick(int steps = 1) => _state = _bridge.Tick(steps);

    /// <summary>Re-read the current live rollout state without mutating it.</summary>
    public EngineResult Refresh() => _state = _bridge.GetState();

    public void Dispose() => _bridge.Dispose();
}
