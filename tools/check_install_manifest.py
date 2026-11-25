#!/usr/bin/env python3
"""Reject installation/uninstallation manifests outside Grassy's namespace."""
import argparse
from pathlib import Path


def checked_paths(manifest, prefix):
    prefix = prefix.resolve()
    paths = []
    for line in manifest.read_text(encoding="utf-8").splitlines():
        if not line:
            continue
        path = Path(line)
        if not path.is_absolute() or ".." in path.parts:
            raise RuntimeError(f"Invalid installation path: {path}")
        try:
            relative = path.relative_to(prefix)
            real_parent = path.parent.resolve().relative_to(prefix)
        except ValueError as error:
            raise RuntimeError(f"Installation path outside prefix {prefix}: {path}") from error
        if not any("grassy" in part.casefold() for part in relative.parts):
            raise RuntimeError(f"Unnamespaced installation path: {path}")
        resolved_relative = real_parent / path.name
        if not any("grassy" in part.casefold() for part in resolved_relative.parts):
            raise RuntimeError(f"Path resolves outside Grassy's namespace: {path}")
        if any("klassy" in part.casefold() for part in resolved_relative.parts):
            raise RuntimeError(f"Path would touch Klassy: {path}")
        paths.append(path)
    if not paths:
        raise RuntimeError("Empty Grassy installation manifest")
    return paths


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--prefix", type=Path, default=Path("/usr"))
    parser.add_argument("--remove", action="store_true")
    args = parser.parse_args()
    paths = checked_paths(args.manifest, args.prefix)
    if args.remove:
        for path in reversed(paths):
            if path.is_symlink() or path.is_file():
                path.unlink()
            elif path.exists():
                raise RuntimeError(f"Refusing to remove non-file: {path}")
        print(f"Removed {len(paths)} manifest entries; no Klassy files touched.")
    else:
        print(f"Validated {len(paths)} Grassy-only installation paths.")


if __name__ == "__main__":
    main()
