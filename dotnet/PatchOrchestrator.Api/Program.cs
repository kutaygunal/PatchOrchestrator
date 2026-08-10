using System.Collections.Concurrent;
using Microsoft.AspNetCore.Mvc;

var builder = WebApplication.CreateBuilder(args);

// Enable OpenAPI document generation (documented contract).
builder.Services.AddOpenApi();

var app = builder.Build();

// Serve the OpenAPI document at /openapi/v1.json and Swagger UI at /swagger.
app.MapOpenApi();
app.UseSwaggerUI(options =>
{
    options.SwaggerEndpoint("/openapi/v1.json", "PatchOrchestrator API v1");
});

// In-memory schedule store (no database for this phase).
var schedules = new ConcurrentDictionary<string, Schedule>();

// --- Health ---
app.MapGet("/api/health", () => Results.Ok(new { status = "ok" }));

// --- Create schedule ---
app.MapPost("/api/schedules", (CreateScheduleRequest request) =>
{
    if (string.IsNullOrWhiteSpace(request.Id))
    {
        return Results.BadRequest(new { error = "id is required" });
    }

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

// --- Status query ---
app.MapGet("/api/schedules/{id}/status", (string id) =>
{
    if (!schedules.TryGetValue(id, out var schedule))
    {
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }
    return Results.Ok(new { id = schedule.Id, status = schedule.Status });
});

app.Run();

IResult ApplyTransition(string id, string newStatus)
{
    if (!schedules.TryGetValue(id, out var schedule))
    {
        return Results.NotFound(new { error = $"schedule '{id}' not found" });
    }
    schedule.Status = newStatus;
    return Results.Ok(new { id = schedule.Id, status = schedule.Status });
}

public record CreateScheduleRequest(string Id, string? Package, string? GroupId);

public class Schedule
{
    public string Id { get; set; } = "";
    public string? Package { get; set; }
    public string? GroupId { get; set; }
    public string Status { get; set; } = "pending";
}
