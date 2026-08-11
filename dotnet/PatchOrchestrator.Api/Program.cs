using System.Collections.Concurrent;
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

    // Start a live engine session for this schedule so the control endpoints
    // (pause/resume/rollback) can mutate real engine state.
    sessions[request.Id] = new EngineSession(bridge, CreateDefaultRequest());

    return Results.Created($"/api/schedules/{request.Id}", schedule);
});

// --- Pause / Resume / Rollback (live EngineSession) ---
app.MapPost("/api/schedules/{id}/pause", (string id) => ControlSession(id, s => s.Pause()));
app.MapPost("/api/schedules/{id}/resume", (string id) => ControlSession(id, s => s.Resume()));
app.MapPost("/api/schedules/{id}/rollback", (string id) => ControlSession(id, s => s.Rollback()));

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
IResult ControlSession(string id, Func<EngineSession, EngineResult> action)
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

// Default live fleet configuration used when a schedule is created.
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

public record CreateScheduleRequest(string Id, string? Package, string? GroupId);

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
}
