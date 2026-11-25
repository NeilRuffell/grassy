# Classic relief rendering

The Titlebar tab provides independent switches for active and inactive two-stage gradients, raised small buttons, beveled window frames, title shadows, stepped corners and compact geometry, and classic vector button glyphs. Optional custom decoration colours override KDE titlebar colours without changing the application colour scheme. Frame width applies only when frame bevel is enabled and the KDE border-size choice permits the edge.

The two-stage titlebar and button gradients use HLS lightness/saturation shading and coefficients from the [Human reference recreation](https://github.com/NeilRuffell/ubuntu-9.04-human-kdecoration). The second revision adds a stronger top highlight, step-boundary tracing and extra inner frame lines. Classic gradients take precedence over the inherited linear gradients when enabled. Button relief requires a small square or rounded-square button shape with a visible background.

Classic geometry uses the five-step corner silhouette and a 24 logical-pixel minimum titlebar with small buttons, subject to the selected font height and titlebar margins. Small buttons have a 24 logical-pixel padded area and an approximately 22 by 18 logical-pixel raised surface. The classic glyph option draws the minimize, maximize, restore and close silhouettes at 10 logical pixels with vector paths and relief shadows. Neither option changes button actions. Other buttons use the existing icon renderer.

## No raster decoration assets

Classic relief, stepped paths and classic glyphs use QPainter primitives and gradients only. There are no PNG, SVG or other image assets in the classic renderer. KWin still uses a procedurally generated shadow texture, and the application-provided menu icon and the user's explicitly selected icon theme are outside this guarantee. Existing project artwork and previews outside the decoration renderer are unchanged.

Load the bundled **Human Jaunty** preset for the source-reference palette, or **Human Orange** for an editable orange approximation. Both include decoration colours without requiring a KDE colour scheme change. Existing users' bundled presets refresh when opening settings without replacing non-bundled presets of the same name. Reload the preset to apply newly added options.

## Remaining accuracy limits

The reference specifies fixed 24 px titlebar and 26 px button hitbox, 5 px frame, stepped corner pixels and bitmap button glyphs. Grassy approximates the original vector glyph silhouettes without distributing raster assets, preserves readable font sizes and native window button geometry, and still relies on KWin's rounded border-radius API and generated shadow texture. Exact per-pixel bevels and maximized-window geometry remain unverified. The orange palette is estimated. No full KDE build or live visual comparison has been performed.

## Validation

Run the Python source-level tests, build and test on Plasma 6 with Klassy installed. Reload both presets and test active/inactive, normal/hover/pressed, maximize/restore, tiled and shaded windows, at 100%, 125%, 150% and 200% scaling. Verify that title text and glyphs remain crisp and that corner/outline/shadow geometry is coherent. Load an older preset to confirm the new switches default to off.
