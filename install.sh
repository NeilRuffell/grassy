#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build="$root/build-grassy"
# Never reuse or uninstall the upstream-shaped build/ directory.
cmake -S "$root" -B "$build" -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    -DKDE_INSTALL_USE_QT_SYS_PATHS=ON "$@"
cmake --build "$build" --parallel "$(getconf _NPROCESSORS_ONLN)"
stage=$(mktemp -d)
trap 'rm -rf -- "$stage"' EXIT HUP INT TERM
# Stage without root privileges first; refuse to install a colliding manifest.
DESTDIR="$stage" cmake --install "$build"
prefix=$(sed -n 's/^CMAKE_INSTALL_PREFIX:PATH=//p' "$build/CMakeCache.txt")
test -n "$prefix"
python3 "$root/tools/check_install_manifest.py" "$build/install_manifest.txt" --prefix "$prefix"
sudo cmake --install "$build"
