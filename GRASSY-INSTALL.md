# Grassy: side-by-side installation

Grassy is a fork of [Klassy](https://github.com/paulmcauley/klassy) by Paul A McAuley. It uses the same upstream-shaped source files to make future rebases manageable. CMake generates an isolated source tree in `build-grassy/grassy-source` and compiles that tree, never the unmodified Klassy identities. Python 3 is required at configure time.

## Install

On a system with the upstream Klassy build dependencies installed, run:

```sh
./install.sh -DBUILD_QT5=OFF
```

Remove `-DBUILD_QT5=OFF` if Qt 5 development dependencies are available and you want the Qt 5 application style. The installer uses a separate build directory, performs an unprivileged staged install, checks the install manifest, and only then installs into the system prefix. It never automatically uninstalls a previous Klassy or Grassy build. Do not reuse an old upstream-shaped `build/` directory.

Select **Grassy** in System Settings → Colors & Themes → Window Decorations. Grassy Settings is a separate application; run `grassy-settings` if needed.

The generated identities include `org.kde.grassy`, `kcm_grassydecoration`, `libgrassycommon6`, `grassy-settings`, `Grassy` Qt styles and C++ namespace, and `~/.config/grassy/grassyrc`. D-Bus signals, presets directory, icon theme names, Plasma theme IDs, translation domains, CMake package and all four colour-scheme filenames are separately named. The original Klassy preset interchange header, extension and icon-inheritance keys remain compatible; saved presets live in separate configuration directories. Existing Klassy configuration is not migrated automatically.

KWin's selected decoration, button placement, colour scheme and other system-wide desktop settings are inherently shared KDE settings, not independently stored by a decoration.

## Uninstall

Switch to another decoration/application style first, then run:

```sh
./uninstall.sh
```

The uninstaller requires the Grassy install manifest and rejects any path outside the Grassy namespace. It does not use the original `build/` directory, reconstruct a missing manifest, or remove Klassy files. User configuration is retained.

## Validation before merging

1. Run `python3 -m unittest discover -s tests -p 'test_*.py'`.
2. Build the branch and stage installation with Klassy still installed.
3. Compare the two install manifests (no common owned file paths).
4. Confirm both window decorations and both Qt application styles are listed and load correctly.
5. Change the active and inactive gradient settings in Grassy; confirm Klassy's configuration file and preset store are unchanged.
6. Update or reinstall Klassy and verify Grassy still loads; uninstall Grassy and verify Klassy still loads.

The namespace generation and manifest guard are designed to prevent file collisions; the full KDE runtime check is still required. Existing Klassy artwork is retained for now.
