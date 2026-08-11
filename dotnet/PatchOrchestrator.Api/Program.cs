using System.Collections.Concurrent;
using System.Text.Json;
using System.Threading.Channels;
using Microsoft.AspNetCore.Mvc;
using PatchOrchestrator.Api;

var builder = WebApplication.CreateBuilder(args);

// Enable OpenAPI document generation (documented contract).
builder.Services.AddOpenApi();

// Register the API <-> engine bridge (drives the Python simulation engine).
builder.Services.AddSingleton<IEngineBridge, EngineBridge>();

var app = builder.Build();

// Structured logging for each request (timestamp + level + message) via the
// Microsoft.Extensions.Logging pipeline already wired by the web host.
var logger = app.Services.GetRequiredService<ILogger<Program>>();

// Serve the OpenAPI document at /openapi/v1.json and Swagger UI at /swagger.
app.MapOpenApi();
app.UseSwaggerUI(options =>
{
    options.SwaggerEndpoint("/openapi/v1.json", "PatchOrchestrator API v1");
});

// In-memory schedule store (no database for this phase).
var schedules = new ConcurrentDictionary<string, Schedule>();

// Live engine sessions keyed by schedule id (Sprint 13 / B3). Each schedule
// owns one long-lived EngineSession so pause/resume/rollback mutate the live
// engine state in place instead of returning a status string.
var sessions = new ConcurrentDictionary<string, EngineSession>();

// Per-schedule broadcast channels (Sprint 15 / B5). Each schedule owns an
// unbounded Channel<EngineResult> that the status/stream endpoint reads from.
// Every live state mutation (pause/resume/rollback/tick) publishes the new
// engine result to the channel so connected clients receive live updates.
var streams = new ConcurrentDictionary<string, Channel<EngineResult>>();

// --- Health ---
app.MapGet("/api/health", () =>
{
    logger.LogInformation("Health check requested");
    return Results.Ok(new { status = "ok" });
});

// --- Create schedule ---
app.MapPost("/api/schedules", (CreateScheduleRequest request, IEngineBridge bridge) =>
{
    if (string.IsNullOrWhiteSpace(request.Id))
    {
        logger.LogWarning("Rejected schedule create with missing id");
        return Results.BadRequest(new { error = "id is required" });
    }

    logger.LogInformation("Creating schedule {Id} (package {Package}, group {GroupId})",
        request.Id, request.Package, request.GroupId);

    var schedule = new Schedule
    {
        Id = request.Id,
        Package = request.Package,
        GroupId = request.GroupId,
        Status = "running"
    };

    schedules[request.Id] = schedule;

    // Record the schedule-creation operator action (Sprint 33 / E2).
    schedule.Actions.Add(new ActionLogEntry("schedule", request.Id, DateTimeOffset.UtcNow, "ok"));

    // Start a live engine session for this schedule so the control endpoints
    // (pause/resume/rollback) can mutate real engine state. When configured
    // fleet size / failure rate / seed are supplied (Sprint 31 / D7), build the
    // engine request from those values; otherwise fall back to the default.
    sessions[request.Id] = new EngineSession(bridge, BuildRequestFromConfig(request));

    // Open a broadcast channel so the status/stream endpoint can deliver live
    // state changes to connected clients.
    streams[request.Id] = Channel.CreateUnbounded<EngineResult>();

    return Results.Created($"/api/schedules/{request.Id}", schedule);
});

// --- Pause / Resume / Rollback (live EngineSession) ---
app.MapPost("/api/schedules/{id}/pause", (string id) => ControlSession(id, "pause", s => s.Pause()));
app.MapPost("/api/schedules/{id}/resume", (string id) => ControlSession(id, "resume", s => s.Resume()));
app.MapPost("/api/schedules/{id}/rollback", (string id) => ControlSession(id, "rollback", s => s.Rollback()));

// --- Tick (advance the live EngineSession deterministically) ---
app.MapPost("/api/schedules/{id}/tick", (string id, TickRequest? request) =>
    TickSession(id, request?.Steps ?? 1));

// --- Simulate (drive the Python engine through the bridge) ---
app.MapPost("/api/schedules/{id}/simulate", (string id, SimulateRequest request, IEngineBridge bridge) =>
{
    if (!schedules.ContainsKey(id))
    {
        logger.LogWarning("Simulate requested for unknown schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }

    logger.LogInformation(
        "Simulating schedule {Id} with {Count} endpoint(s), seed {Seed}",
        id, request.Endpoints.Count, request.Seed);

    var engineRequest = new EngineRequest(
        request.Endpoints.Select(e => new EngineEndpointRequest(e.Id, e.FailureRate)).ToList(),
        request.Seed);

    try
    {
        var result = bridge.Run(engineRequest);
        logger.LogInformation("Simulation for schedule {Id} completed", id);
        return Results.Ok(new
        {
            endpoints = result.Endpoints.Select(e => new { e.Id, e.State, e.Progress }),
        });
    }
    catch (Exception ex)
    {
        logger.LogError(ex, "Simulation for schedule {Id} failed", id);
        return Results.Problem("Simulation engine failed", statusCode: 500);
    }
});

// --- Action log query (Sprint 33 / E2) ---
// Returns the recorded operator actions for a schedule in chronological order.
// 404 for an unknown schedule id, matching the other endpoints' style.
app.MapGet("/api/schedules/{id}/actions", (string id) =>
{
    if (!schedules.TryGetValue(id, out var schedule))
    {
        logger.LogWarning("Actions requested for unknown schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }
    logger.LogInformation("Actions requested for schedule {Id} ({Count} entries)", id, schedule.Actions.Count);
    return Results.Ok(schedule.Actions.OrderBy(a => a.Timestamp));
});

// --- Status query ---
app.MapGet("/api/schedules/{id}/status", (string id) =>
{
    if (!schedules.TryGetValue(id, out var schedule))
    {
        logger.LogWarning("Status requested for unknown schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }
    logger.LogInformation("Status requested for schedule {Id} ({Status})", id, schedule.Status);
    return Results.Ok(new { id = schedule.Id, status = schedule.Status });
});

// --- Status stream (SSE, Sprint 15 / B5) ---
// Delivers live state changes to clients as they occur. The stream opens with
// the current state as a baseline, then emits one SSE event per live mutation
// (pause/resume/rollback/tick). Returns 404 for an unknown schedule id and
// handles client disconnect gracefully (the request-aborted token cancels the
// channel read, which the framework treats as a normal disconnect).
app.MapGet("/api/schedules/{id}/status/stream", async (string id, HttpResponse response, CancellationToken ct) =>
{
    if (!schedules.TryGetValue(id, out _))
    {
        logger.LogWarning("Stream requested for unknown schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }

    if (!streams.TryGetValue(id, out var channel))
    {
        logger.LogWarning("No stream channel for schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' has no stream" });
    }

    logger.LogInformation("Stream opened for schedule {Id}", id);
    response.Headers.ContentType = "text/event-stream";
    response.Headers.CacheControl = "no-cache";

    // Emit the current state immediately so the client has a baseline.
    if (sessions.TryGetValue(id, out var session))
    {
        await WriteStreamEventAsync(response, id, session.State, ct);
    }

    // Emit one event per live state change until the client disconnects.
    await foreach (var result in channel.Reader.ReadAllAsync(ct))
    {
        await WriteStreamEventAsync(response, id, result, ct);
    }

    return Results.Empty;
});

// Dispose all live engine sessions (and their Python subprocesses) on shutdown.
app.Lifetime.ApplicationStopped.Register(() =>
{
    foreach (var session in sessions.Values)
    {
        session.Dispose();
    }
    sessions.Clear();
});

app.Run();

// Invoke a live EngineSession operation for a schedule and reflect the new
// engine state back onto the schedule record. Returns 200 with the updated
// state, 404 for an unknown schedule id, and 500 if the engine fails.
IResult ControlSession(string id, string actionName, Func<EngineSession, EngineResult> action)
{
    if (!schedules.TryGetValue(id, out var schedule))
    {
        logger.LogWarning("Control requested for unknown schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }

    if (!sessions.TryGetValue(id, out var session))
    {
        logger.LogWarning("No live session for schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' has no live session" });
    }

    try
    {
        var result = action(session);
        schedule.Status = StatusFromResult(result);
        PublishState(id, result);
        // Record the operator action with the resulting state (Sprint 33 / E2).
        schedule.Actions.Add(new ActionLogEntry(actionName, id, DateTimeOffset.UtcNow, schedule.Status));
        logger.LogInformation("Schedule {Id} control -> {Status}", id, schedule.Status);
        return Results.Ok(new { id = schedule.Id, status = schedule.Status });
    }
    catch (Exception ex)
    {
        logger.LogError(ex, "Control for schedule {Id} failed", id);
        return Results.Problem("Engine control failed", statusCode: 500);
    }
}

// Invoke a live EngineSession tick for a schedule and reflect the new engine
// state back onto the schedule record. Returns 200 with the updated state
// (including per-endpoint progress), 404 for an unknown schedule id, and 500
// if the engine fails. The tick is deterministic (seeded via the session).
IResult TickSession(string id, int steps)
{
    if (!schedules.TryGetValue(id, out var schedule))
    {
        logger.LogWarning("Tick requested for unknown schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }

    if (!sessions.TryGetValue(id, out var session))
    {
        logger.LogWarning("No live session for schedule {Id}", id);
        return Results.NotFound(new { error = $"schedule '{id}' has no live session" });
    }

    try
    {
        var result = session.Tick(steps);
        schedule.Status = StatusFromResult(result);
        PublishState(id, result);
        // Record the tick operator action (Sprint 33 / E2).
        schedule.Actions.Add(new ActionLogEntry("tick", id, DateTimeOffset.UtcNow, schedule.Status));
        logger.LogInformation("Schedule {Id} tick x{Steps} -> {Status}", id, steps, schedule.Status);
        return Results.Ok(new
        {
            id = schedule.Id,
            status = schedule.Status,
            endpoints = result.Endpoints.Select(e => new { e.Id, e.State, e.Progress }),
        });
    }
    catch (Exception ex)
    {
        logger.LogError(ex, "Tick for schedule {Id} failed", id);
        return Results.Problem("Engine tick failed", statusCode: 500);
    }
}

// Publish a live engine result to a schedule's broadcast channel so connected
// status/stream clients receive the state change.
void PublishState(string id, EngineResult result)
{
    if (streams.TryGetValue(id, out var channel))
    {
        channel.Writer.TryWrite(result);
    }
}

// Write one SSE event carrying the schedule id, derived status, and per-endpoint
// state/progress to the response body.
static async Task WriteStreamEventAsync(HttpResponse response, string id, EngineResult result, CancellationToken ct)
{
    var payload = new
    {
        id,
        status = StatusFromResult(result),
        endpoints = result.Endpoints.Select(e => new { e.Id, e.State, e.Progress }),
    };
    var json = JsonSerializer.Serialize(payload);
    await response.WriteAsync($"data: {json}\n\n", ct);
    await response.Body.FlushAsync(ct);
}

// Derive a single schedule status string from the live engine result.
static string StatusFromResult(EngineResult result)
{
    if (result.RolledBack)
    {
        return "rolled_back";
    }

    var states = result.Endpoints.Select(e => e.State).Distinct().ToList();
    return states.Count == 1 ? states[0] : "running";
}

// Build an EngineRequest from configured demo values when supplied (Sprint 31 /
// D7): endpoints count = fleet size, each endpoint's failure rate, and the seed.
// Falls back to a default request when no configuration is provided.
static EngineRequest BuildRequestFromConfig(CreateScheduleRequest request)
{
    if (request.FleetSize.HasValue && request.FailureRate.HasValue && request.Seed.HasValue)
    {
        return EngineRequestFactory.Build(
            request.FleetSize.Value, request.FailureRate.Value, request.Seed.Value);
    }

    return CreateDefaultRequest();
}

// Default live fleet configuration used when a schedule is created without
// configured D1–D3 values.
static EngineRequest CreateDefaultRequest() => new(
    new List<EngineEndpointRequest>
    {
        new("ep-1", 0.1),
        new("ep-2", 0.0),
        new("ep-3", 0.3),
    },
    Seed: 42);

// Expose the generated Program class to the integration test project so it
// can bootstrap the web host via WebApplicationFactory<Program>.
public partial class Program { }

public record CreateScheduleRequest(
    string Id, string? Package, string? GroupId,
    int? FleetSize = null, double? FailureRate = null, int? Seed = null);

public record SimulateEndpointRequest(string Id, double FailureRate);
public record SimulateRequest(int Seed, IReadOnlyList<SimulateEndpointRequest> Endpoints);

/// <summary>Optional body for the tick endpoint; Steps defaults to 1.</summary>
public record TickRequest(int Steps);

public class Schedule
{
    public string Id { get; set; } = "";
    public string? Package { get; set; }
    public string? GroupId { get; set; }
    public string Status { get; set; } = "pending";

    /// <summary>Recorded operator actions in chronological order (Sprint 33 / E2).</summary>
    public List<ActionLogEntry> Actions { get; } = new();
}
