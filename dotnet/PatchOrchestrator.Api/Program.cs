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

// --- Health ---
app.MapGet("/api/health", () =>
{
    logger.LogInformation("Health check requested");
    return Results.Ok(new { status = "ok" });
});

// --- Create schedule ---
app.MapPost("/api/schedules", (CreateScheduleRequest request) =>
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
        Status = "pending"
    };

    schedules[request.Id] = schedule;
    return Results.Created($"/api/schedules/{request.Id}", schedule);
});

// --- Pause / Resume / Rollback ---
app.MapPost("/api/schedules/{id}/pause", (string id) => ApplyTransition(id, "paused"));
app.MapPost("/api/schedules/{id}/resume", (string id) => ApplyTransition(id, "running"));
app.MapPost("/api/schedules/{id}/rollback", (string id) => ApplyTransition(id, "rolled-back"));

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

app.Run();

IResult ApplyTransition(string id, string newStatus)
{
    if (!schedules.TryGetValue(id, out var schedule))
    {
        logger.LogWarning("Transition to {NewStatus} requested for unknown schedule {Id}",
            newStatus, id);
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }
    logger.LogInformation("Schedule {Id}: {OldStatus} -> {NewStatus}",
        id, schedule.Status, newStatus);
    schedule.Status = newStatus;
    return Results.Ok(new { id = schedule.Id, status = schedule.Status });
}

public record CreateScheduleRequest(string Id, string? Package, string? GroupId);

public record SimulateEndpointRequest(string Id, double FailureRate);
public record SimulateRequest(int Seed, IReadOnlyList<SimulateEndpointRequest> Endpoints);

public class Schedule
{
    public string Id { get; set; } = "";
    public string? Package { get; set; }
    public string? GroupId { get; set; }
    public string Status { get; set; } = "pending";
}
