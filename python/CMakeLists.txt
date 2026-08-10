# PatchOrchestrator — Python simulation engine (Phase 1 stub).
#
# The engine is a pure-Python package. In Phase 1 we only register a
# placeholder custom target so the build graph is stable. Phase 3 replaces
# this with the real engine package and pytest wiring.

# Placeholder target so `cmake --build` has something to do for python/.
add_custom_target(python-engine
    COMMAND ${CMAKE_COMMAND} -E echo "python-engine: placeholder (Phase 3 adds the simulation engine)"
    COMMENT "Python simulation engine stub"
)
