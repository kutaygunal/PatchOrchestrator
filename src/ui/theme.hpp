// PatchOrchestrator — shared application theme.
//
// A single, dark, modern visual identity applied to every desktop window in
// the product (dashboard, control panel, schedule editor, demo hub) so the
// suite reads as one cohesive commercial tool instead of a set of
// default-Qt-styled utilities. This is the one place that owns the palette,
// so every window stays visually consistent by construction: a window picks
// up the shared look simply by calling Theme::apply() before it is
// constructed, with no per-window styling of its own.
//
// Deliberately independent of the state->color mapping owned by StateBadge
// (running/paused/succeeded/...): that palette is semantic status color and
// must stay stable for tests and operator muscle memory. This theme covers
// everything else — chrome, typography, spacing, and the brand accent.

#ifndef PATCHORCHESTRATOR_UI_THEME_HPP
#define PATCHORCHESTRATOR_UI_THEME_HPP

class QApplication;

namespace Theme
{

// Apply the Fusion style, a dark QPalette, and the shared stylesheet to the
// given application. Call once, immediately after constructing QApplication
// and before constructing any windows, so every widget picks it up.
void apply(QApplication &app);

} // namespace Theme

#endif // PATCHORCHESTRATOR_UI_THEME_HPP
