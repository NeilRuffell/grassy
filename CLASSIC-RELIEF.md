# Classic relief rendering

The Titlebar tab provides independent switches for active and inactive two-stage gradients, raised small buttons, beveled window frames, title shadows, stepped corners and compact geometry, and classic vector button glyphs. Optional custom decoration colours override KDE titlebar colours without changing the application colour scheme. Frame width applies only when frame bevel is enabled and the KDE border-size choice permits the edge.

The two-stage titlebar and button gradients use HLS lightness/saturation shading and coefficients from the [Human reference recreation](https://github.com/NeilRuffell/ubuntu-9.04-human-kdecoration). The second revision adds a stronger top highlight, step-boundary tracing and extra inner frame lines. Classic gradients take precedence over the inherited linear gradients when enabled. Button relief requires a small square or rounded-square button shape with a visible background.

Classic geometry uses the five-step corner silhouette and a 24 logical-pixel minimum titlebar with small buttons, subject to the selected font height and titlebar margins. Small buttons have a 24 logical-pixel padded area and an approximately 22 by 18 logical-pixel raised surface. The classic glyph option draws the minimize, maximize, restore and close silhouettes at 10 logical pixels with vector paths and relief shadows. Neither option changes button actions. Other buttons use the existing icon renderer.

## No raster decoration assets

Classic relief, stepped paths and classic glyphs use QPainter primitives and gradients only. There are no PNG, SVG or other image assets in the classic renderer. KWin still uses a procedurally generated shadow texture, and the application-provided menu icon and the user's explicitly selected icon theme are outside this guarantee. Existing project artwork and previews outside the decoration renderer are unchanged.

## Colour scheme integration

Load the single bundled **Human** preset. It contains rendering and geometry settings, not a fixed palette. Active and inactive titlebar and title text colours follow Klassy's existing KDE colour handling; the frame follows the window frame colour and button gloss derives from the titlebar colour. Bevels and shadows are calculated from those base colours. Select the desired KDE colour scheme separately. Manual decoration colour overrides remain available, but are disabled in the bundled Human preset.

The old bundled Human Jaunty and Human Orange presets are retired. Existing bundled presets refresh when opening settings; user-saved presets are preserved. Stale installed copies of the two retired preset files are ignored on bundled import because CMake does not remove obsolete files when upgrading. Reload Human to switch an already-applied old palette to scheme-driven colours. Existing window-specific rules referencing a retired bundled preset must select Human instead.

## Remaining accuracy limits

The reference specifies fixed 24 px titlebar and 26 px button hitbox, 5 px frame, stepped corner pixels and bitmap button glyphs. Grassy approximates the original vector glyph silhouettes without distributing raster assets, preserves readable font sizes and native window button geometry, and still relies on KWin's rounded border-radius API and generated shadow texture. Exact per-pixel bevels and maximized-window geometry remain unverified. Exact historical colours depend on the selected KDE colour scheme. This palette consolidation has not been tested in a live KDE session.

## Validation

Run the Python source-level tests, build and test on Plasma 6 with Klassy installed. Load Human, switch between light, dark and differently coloured KDE schemes without reloading the preset, and test active/inactive, normal/hover/pressed, maximize/restore, tiled and shaded windows, at 100%, 125%, 150% and 200% scaling. Verify that title text and glyphs remain crisp and that corner/outline/shadow geometry is coherent. Load an older preset to confirm the new switches default to off.
