# Classic relief rendering

The Titlebar tab provides independent switches for active and inactive two-stage gradients, raised small buttons, beveled window frames, title shadows, stepped corners and compact geometry, and classic vector button glyphs. Optional custom decoration colours override KDE titlebar colours without changing the application colour scheme. Frame width applies only when frame bevel is enabled and the KDE border-size choice permits the edge.

The two-stage titlebar and button gradients use HLS lightness/saturation shading and coefficients from the [Human reference recreation](https://github.com/NeilRuffell/ubuntu-9.04-human-kdecoration). The second revision adds a stronger top highlight, step-boundary tracing and extra inner frame lines. Classic gradients take precedence over the inherited linear gradients when enabled. The two stages are painted together using one four-stop gradient with closely spaced midpoint stops, avoiding an antialiased seam between separate gradient rectangles even when the frame colour differs. Button relief requires a small square or rounded-square button shape with a visible background.

Classic geometry uses the five-step corner silhouette and scalable small-button proportions. The native Icon size (or System icon size for the system-icon style) controls the size selection; Button Size & Spacing controls the background scale independently. At the Human preset's SmallMedium and 112% background defaults (1x logical scale), the padded area is 24 logical pixels, the raised surface is approximately 22 by 18 logical pixels, and the classic vector glyphs are 10 logical pixels. Other icon sizes scale these proportions. Titlebar height follows the greater of the padded button area and the selected font height, plus the titlebar margins and any separator. Frame width and corner radius remain independent. Neither option changes button actions. Other buttons use the existing icon renderer.

## No raster decoration assets

Classic relief, stepped paths and classic glyphs use QPainter primitives and gradients only. There are no PNG, SVG or other image assets in the classic renderer. KWin still uses a procedurally generated shadow texture, and the application-provided menu icon and the user's explicitly selected icon theme are outside this guarantee. Existing project artwork and previews outside the decoration renderer are unchanged.

## Colour scheme integration

Load the single bundled **Human** preset. It contains rendering and geometry settings, not a fixed palette. Active and inactive titlebar and title text colours follow Klassy's existing KDE colour handling; the frame follows the window frame colour and button gloss derives from the titlebar colour. Bevels and shadows are calculated from those base colours. Select the desired KDE colour scheme separately. Manual decoration colour overrides remain available, but are disabled in the bundled Human preset.

The old bundled Human Jaunty and Human Orange presets are retired. Existing bundled presets refresh when opening settings; user-saved presets are preserved. Stale installed copies of the two retired preset files are ignored on bundled import because CMake does not remove obsolete files when upgrading. Reload Human to switch an already-applied old palette to scheme-driven colours. Existing window-specific rules referencing a retired bundled preset must select Human instead.

## Remaining accuracy limits

The reference specifies fixed 24 px titlebar and 26 px button hitbox, 5 px frame, stepped corner pixels and bitmap button glyphs. Grassy approximates the original vector glyph silhouettes without distributing raster assets, preserves readable font sizes and native window button geometry, and still relies on KWin's rounded border-radius API and generated shadow texture. Exact per-pixel bevels and maximized-window geometry remain unverified. Exact historical colours depend on the selected KDE colour scheme. This palette consolidation has not been tested in a live KDE session.

## Validation

Run the Python source-level tests, build and test on Plasma 6 with Klassy installed. Load Human, switch between light, dark and differently coloured KDE schemes without reloading the preset, and test active/inactive, normal/hover/pressed, maximize/restore, tiled and shaded windows, at 100%, 125%, 150% and 200% scaling. Verify that title text and glyphs remain crisp and that corner/outline/shadow geometry is coherent. Compare titlebars with Use titlebar colour for all window borders both enabled and disabled, especially at odd titlebar heights and fractional scaling: the midpoint should show only the intended change in gradient shade, not a line of frame colour. Change Icon size and background scale independently, checking the glyph, menu icon, hit area and titlebar height (font height can limit shrinking). The default Human proportions should remain unchanged. Load an older preset to confirm the new switches default to off.
