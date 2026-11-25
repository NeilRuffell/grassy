#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build="$root/build-grassy"
manifest="$build/install_manifest.txt"
if [ ! -f "$manifest" ] || [ ! -f "$build/CMakeCache.txt" ]; then
    echo "No Grassy installation manifest found. Refusing to infer or remove files." >&2
    exit 1
fi
prefix=$(sed -n 's/^CMAKE_INSTALL_PREFIX:PATH=//p' "$build/CMakeCache.txt")
test -n "$prefix"
python3 "$root/tools/check_install_manifest.py" "$manifest" --prefix "$prefix"
sudo python3 "$root/tools/check_install_manifest.py" "$manifest" --prefix "$prefix" --remove
