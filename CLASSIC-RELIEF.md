# Classic relief rendering

The Titlebar tab provides independent switches for active and inactive two-stage gradients, raised small buttons, beveled window frames, and title shadows. Optional custom decoration colours override KDE titlebar colours without changing the application colour scheme. Frame width applies only when frame bevel is enabled and the KDE border-size choice permits the edge.

The two-stage titlebar and button gradients use HLS lightness/saturation shading and coefficients from the [Human reference recreation](https://github.com/NeilRuffell/ubuntu-9.04-human-kdecoration). Classic gradients take precedence over the inherited linear gradients when enabled. Button relief requires a small square or rounded-square button shape with a visible background. The native icon renderer and button operations remain unchanged.

Load the bundled **Human Jaunty** preset for the source-reference palette, or **Human Orange** for an editable orange approximation. Both include decoration colours without requiring a KDE colour scheme change. Existing users' bundled presets refresh on opening settings, without replacing non-bundled presets of the same name.

## Remaining accuracy limits

The reference specifies a 24 px titlebar, 26 px button hitbox, 5 px frame, stepped corner pixels and bitmap button glyphs. This change implements configurable frame thickness, gradients and bevels, but retains Grassy's font-based titlebar/button sizing, rounded clipping, vector glyphs and maximized-window geometry. Bevel lines approximate the reference rather than reproduce every corner pixel operation. Original Ubuntu package assets could not be downloaded in this environment; the orange palette is estimated.

## Validation

Build and test on Plasma 6 with Klassy installed. Load both presets, verify active/inactive windows and normal/hover/pressed buttons, tiled/maximized/shaded windows and fractional scaling; restart KWin and verify settings persist. Load an older preset to confirm that all new effects default to off. No full KDE build or live visual comparison has been performed.
