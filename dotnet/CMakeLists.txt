# PatchOrchestrator — .NET REST API (Phase 1 stub).
#
# The API is an ASP.NET Core service built with `dotnet build`. In Phase 1 we
# only register a placeholder custom target so the build graph is stable.
# Phase 5 replaces this with the real API project and build step.

# Placeholder target so `cmake --build` has something to do for dotnet/.
add_custom_target(dotnet-api
    COMMAND ${CMAKE_COMMAND} -E echo "dotnet-api: placeholder (Phase 5 adds the REST API)"
    COMMENT ".NET REST API stub"
)
