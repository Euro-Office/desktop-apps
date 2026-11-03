#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Euro-Office contributors
# SPDX-License-Identifier: AGPL-3.0-only

# Configures and builds the desktop editors on Linux (win-linux/) and macOS
# (desktop-sdk's ascdocumentscore and the CEF helpers; the app itself is
# built with Xcode afterwards). The Dockerfile, CI and DesktopEditors build
# through this script. Windows: build.ps1.
#
# Core isn't built as part of this project: it links a core package. That is
# the prebuilt one for the core checkout when the build cache has it, else
# one built by core/scripts/build.sh. A core checkout with local changes is
# always built (see core/scripts/core_package.py).
#
# Usage: scripts/build.sh [options] [-- extra cmake arguments]
#   --build-dir DIR     build directory (default: build); the desktop build
#                       is in <DIR>/desktop, a core built here in <DIR>/core
#   --core DIR          core checkout (default: ../core)
#   --desktop-sdk DIR   desktop-sdk checkout (default: ../desktop-sdk)
#   --core-package DIR  use this core package instead of fetching or
#                       building one
#   --tests             also build the unit tests (run them with ctest)
#   --ccache            compile through ccache
#
# Environment: VCPKG_ROOT; CORE_PYTHON, the Python for the build scripts
# (default: python3); EO_CORE_CACHE: auto (default), off (always build core)
# or require (fail without a prebuilt core).

set -euo pipefail

apps="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
core="$apps/../core"
sdk="$apps/../desktop-sdk"
build_dir=build
package=
tests=OFF
ccache=

usage() { sed -n '/^# Usage/,/^# Environment/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit "$1"; }

while [ $# -gt 0 ]; do
    case "$1" in
        --build-dir)    build_dir="$2"; shift 2 ;;
        --core)         core="$2"; shift 2 ;;
        --desktop-sdk)  sdk="$2"; shift 2 ;;
        --core-package) package="$2"; shift 2 ;;
        --tests)        tests=ON; shift ;;
        --ccache)       ccache=1; shift ;;
        -h|--help)      usage 0 ;;
        --)             shift; break ;;
        *)              echo "unknown option: $1" >&2; usage 1 ;;
    esac
done

: "${VCPKG_ROOT:?VCPKG_ROOT must point at vcpkg}"
python="${CORE_PYTHON:-python3}"
mkdir -p "$build_dir"
build_dir="$(cd "$build_dir" && pwd)"
core="$(cd "$core" && pwd)"
sdk="$(cd "$sdk" && pwd)"
# Shared by the core and the desktop build, so each dependency is set up once
third_party="$build_dir/third_party"

case "$(uname -s)-$(uname -m)" in
    Linux-x86_64)            platform=linux-amd64 ;;
    Linux-aarch64)           platform=linux-arm64 ;;
    Darwin-arm64)            platform=macos-arm64 ;;
    *)                       platform= ;;
esac

if [ -z "$package" ]; then
    status=3
    if [ -n "$platform" ]; then
        "$python" "$core/scripts/core_package.py" fetch --core "$core" \
            --platform "$platform" --product desktop --dest "$build_dir/core-prebuilt" \
            && status=0 || status=$?
    fi
    case "$status" in
        0) package="$build_dir/core-prebuilt" ;;
        3) "$core/scripts/build.sh" --build-dir "$build_dir/core" ${ccache:+--ccache} \
               -- -DEO_CORE_3RD_PARTY_DIR="$third_party"
           package="$build_dir/core/package" ;;
        *) exit "$status" ;;
    esac
fi
package="$(cd "$package" && pwd)"

if [ "$(uname -s)" = Darwin ]; then
    source_dir="$sdk/ChromiumBasedEditors/lib"
else
    source_dir="$apps/win-linux"
fi

features="desktop-editors"
[ "$tests" = ON ] && features="desktop-editors;tests"

args=(
    -G Ninja -S "$source_dir" -B "$build_dir/desktop"
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    -DVCPKG_MANIFEST_MODE=ON
    -DVCPKG_MANIFEST_DIR="$core"
    -DVCPKG_MANIFEST_FEATURES="$features"
    -DEO_BUILD_TESTS="$tests"
    -DEO_CORE_PREBUILT_DIR="$package"
    -DEO_CORE_3RD_PARTY_DIR="$third_party"
)
[ -n "${CORE_PYTHON:-}" ] && args+=( -DPYTHON_BIN="$CORE_PYTHON" )
[ -n "$ccache" ] && args+=( -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache )
[ "$(uname -s)" = Darwin ] && args+=( -DBUILD_DESKTOP=1 -DVCPKG_TARGET_TRIPLET="$(uname -m | sed 's/x86_64/x64/')-osx" )

cmake "${args[@]}" "$@"
cmake --build "$build_dir/desktop"
