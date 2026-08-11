# Phase C6 — Test Plan (Sprint 23: Smooth refresh)

Engineer: `senior-engineer-c6`. Tester: `testing-c6`. This is the test plan for using a
`QTimer` + interpolation (or SSE-driven updates) so progress bars animate smoothly without
flicker, driven by the B5 status stream.

## Context

Sprint 18 (C1) added animated progress bars. Sprint 15 (B5) added the real-time status stream.
Sprint 23 combines them: the dashboard uses a `QTimer` + interpolation (or SSE-driven updates)
so progress bars animate smoothly without flicker, driven by the B5 stream.

## Acceptance criteria (from sprint)

1. Smooth animation.
2. No flicker.
3. Driven by B5 stream.

## Test plan

### T1 — Smooth update path

- **Type:** Qt integration test (C++/Qt Test, `QTEST_MAIN`, offscreen platform).
- **Setup:** Construct the dashboard wired to the B5 status stream; drive a state change via
  the stream.
- **Assertions:**
  - Progress bars animate smoothly to the new value (intermediate values, no jump).
  - The update is driven by the B5 stream (not only the poll timer).
  - No flicker (values change monotonically toward the target).
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c6_smooth` (or the equivalent Qt Test binary).

### T2 — No flicker

- **Type:** Qt integration test.
- **Assertions:**
  - Progress bars do not reset/jump backward during an update.
  - Values interpolate monotonically toward the target.
  - The dashboard does not repaint erratically.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c6_no_flicker` (or the equivalent Qt Test binary).

### T3 — Driven by B5 stream

- **Type:** Qt integration test.
- **Assertions:**
  - A state change on the B5 stream triggers a smooth update.
  - The update happens without waiting for the poll timer.
  - Multiple stream events produce a sequence of smooth updates.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c6_stream_driven` (or the equivalent Qt Test binary).

### T4 — Regression

- **Type:** Qt integration test.
- **Assertions:**
  - The dashboard still works with its poll timer (existing behavior preserved).
  - Existing dashboard tests (P8), C1, and B5/B6 tests still pass.
- **Command (bounded, with timeout):**
  `timeout 300 ctest --test-dir build -R c6_regression` (or the equivalent Qt Test binary).

## Notes for the engineer

- Use a `QTimer` + interpolation (or SSE-driven updates) so progress bars animate smoothly
  without flicker, driven by the B5 stream.
- Keep the existing dashboard behavior and C1 progress bars intact so P8/C1/B5/B6 tests keep
  working.
- Follow `docs/working-rules.md`: bounded `ls`/`grep`, hard timeouts, no `find /`.
- Do NOT commit/push — that is the devops agent's job.
