#!/usr/bin/env python3
"""Materialize a deterministic, independently named Grassy build source tree.

Keep upstream file paths in the working tree to reduce future rebase conflicts.
Only the generated tree is passed to CMake; the working tree is never changed.
"""
import argparse
import os
from pathlib import Path
import re
import stat

MARKER = ".grassy-generated"
ROOT_FILES = ("CMakeLists.txt", "KlassyConfig.cmake.in", "AUTHORS")
SOURCE_DIRS = (
    "cmake", "colors", "desktoptheme", "icons", "kdecoration", "kstyle",
    "layout-templates", "libbreezecommon", "look-and-feel", "po", "LICENSES",
)
TEXT_SUFFIXES = {
    ".cpp", ".cc", ".c", ".h", ".hpp", ".cmake", ".in", ".kcfg", ".kcfgc",
    ".ui", ".qrc", ".json", ".desktop", ".themerc", ".colors", ".klpw",
    ".xml", ".svg", ".js", ".po", ".pot", ".sh", ".theme", ".txt",
}
TEXT_NAMES = {"CMakeLists.txt", "colors", "defaults", "plasmarc", "index.theme"}
BRAND = re.compile(r"klassy", re.IGNORECASE)


def rebrand(value):
    def replacement(match):
        word = match.group()
        if word.isupper():
            return "GRASSY"
        if word[0].isupper():
            return "Grassy"
        return "grassy"
    return BRAND.sub(replacement, value).replace("OpalFruits", "GrassyOpalFruits")


def transform_text(value, path):
    # Retain the Klassy preset interchange protocol and its two existing
    # icon-inheritance keys. The on-disk configuration directory is separate.
    value = "\n".join(
        line if re.search(r"SPDX|copyright|distributed under the same license", line, re.I)
        else rebrand(line)
        .replace("Grassy Window Decoration Preset File", "Klassy Window Decoration Preset File")
        .replace("GrassyIconThemeInherits", "KlassyIconThemeInherits")
        .replace("GrassyDarkIconThemeInherits", "KlassyDarkIconThemeInherits")
        .replace("grassyIconThemeInherits", "klassyIconThemeInherits")
        .replace("grassyDarkIconThemeInherits", "klassyDarkIconThemeInherits")
        .replace("https://github.com/paulmcauley/grassy", "https://github.com/paulmcauley/klassy")
        for line in value.split("\n")
    )
    if path.name == "CMakeLists.txt" or path.suffix == ".cmake":
        # The generated project is a subdirectory of the bootstrap project.
        value = re.sub(r"\bCMAKE_SOURCE_DIR\b", "PROJECT_SOURCE_DIR", value)
        value = re.sub(r"\bCMAKE_BINARY_DIR\b", "PROJECT_BINARY_DIR", value)
    if path == Path("CMakeLists.txt"):
        # No Git metadata exists in the generated source tree.
        value = value.replace(
            "kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)",
            "# Git hooks belong to the original source checkout."
        )
    return value


def input_files(source):
    for name in ROOT_FILES:
        path = source / name
        if not path.is_file():
            raise RuntimeError(f"Missing required source file: {path}")
        yield path
    for name in SOURCE_DIRS:
        root = source / name
        if not root.is_dir():
            raise RuntimeError(f"Missing required source directory: {root}")
        for directory, dirs, files in os.walk(root, followlinks=False):
            parent = Path(directory)
            for name in list(dirs):
                path = parent / name
                if path.is_symlink():
                    dirs.remove(name)
                    yield path
            for name in files:
                yield parent / name


def is_text(path):
    return (path.suffix in TEXT_SUFFIXES or path.name in TEXT_NAMES) and (
        path.parts[0] != "LICENSES" and path.name != "AUTHORS"
    )


def stage(source, destination):
    source = source.resolve()
    destination = destination.resolve()
    if destination.name != "grassy-source" or source == destination:
        raise RuntimeError("Destination must be a dedicated grassy-source directory")
    if any(destination == source / part or (source / part) in destination.parents
           for part in SOURCE_DIRS):
        raise RuntimeError("Cannot generate into a source input directory")
    marker = destination / MARKER
    if destination.exists() and not marker.is_file():
        raise RuntimeError(f"Refusing to modify an unowned directory: {destination}")
    destination.mkdir(parents=True, exist_ok=True)
    marker.touch(exist_ok=True)
    expected = {marker}
    for original in sorted(input_files(source)):
        relative = original.relative_to(source)
        target = destination / rebrand(relative.as_posix())
        if target in expected:
            raise RuntimeError(f"Name collision while staging {relative}")
        expected.add(target)
        target.parent.mkdir(parents=True, exist_ok=True)
        if original.is_symlink():
            link = rebrand(os.readlink(original))
            if target.is_symlink() and os.readlink(target) == link:
                continue
            if target.is_symlink() or target.is_file():
                target.unlink()
            target.symlink_to(link)
            continue
        raw = original.read_bytes()
        if is_text(relative):
            raw = transform_text(raw.decode("utf-8"), relative).encode("utf-8")
        if target.is_symlink():
            target.unlink()
        if not target.is_file() or target.read_bytes() != raw:
            target.write_bytes(raw)
        mode = stat.S_IMODE(original.stat().st_mode)
        if stat.S_IMODE(target.stat().st_mode) != mode:
            target.chmod(mode)
    # Remove stale generated paths after a source file is renamed or removed.
    for path in sorted(destination.rglob("*"), key=lambda p: len(p.parts), reverse=True):
        if path in expected:
            continue
        if path.is_symlink() or path.is_file():
            path.unlink()
        elif path.is_dir() and not any(path.iterdir()):
            path.rmdir()
    for path in expected:
        if path.is_symlink() and not path.exists():
            raise RuntimeError(f"Broken generated symlink: {path}")
    print(f"Grassy source isolation: {len(expected)-1} files staged in {destination}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("destination", type=Path)
    args = parser.parse_args()
    stage(args.source, args.destination)
