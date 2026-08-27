#!/usr/bin/env bash
# Regenerates the macOS app icon and title-bar logo PNGs from the Euro-Office
# SVG source art already shipped for win-linux (desktop-apps/win-linux/res/icons).
# Re-run this if those source SVGs ever change; the PNGs it writes are committed
# as the real asset files - Xcode's asset catalog compiler consumes static PNGs,
# nothing regenerates them at build time.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ICONS_DIR="${SCRIPT_DIR}/../win-linux/res/icons"
ASSETS_DIR="${SCRIPT_DIR}/Euro-Office/Images.xcassets"

if ! command -v sips >/dev/null 2>&1; then
    echo "error: sips not found (expected on any Mac)." 1>&2
    exit 1
fi

echo "==> App icon (AppIcon.appiconset) from app-icon-eo.svg"
APP_ICON_SRC="${ICONS_DIR}/app-icon-eo.svg"
APPICONSET="${ASSETS_DIR}/AppIcon.appiconset"
render_square() {
    local size="$1"; local dest="$2"
    sips -s format png -Z "${size}" "${APP_ICON_SRC}" --out "${APPICONSET}/${dest}" >/dev/null
}
render_square 16   "16x16.png"
render_square 32   "32x32.png"
render_square 32   "32x32-1.png"
render_square 64   "64x64.png"
render_square 128  "128x128.png"
render_square 256  "256x256.png"
render_square 256  "256x256-1.png"
render_square 512  "512x512.png"
render_square 512  "512x512-1.png"
render_square 1024 "1024x1024.png"

echo "==> Title-bar logo (Tabs imagesets) from logo-{light,dark}-eo.svg"
TABS_DIR="${ASSETS_DIR}/Tabs"
render_wordmark() {
    local src="$1"; local dest="$2"; local w="$3"; local h="$4"
    sips -s format png -z "${h}" "${w}" "${ICONS_DIR}/${src}" --out "${dest}" >/dev/null
}
# logo-tab-dark.imageset is used in the LIGHT theme -> needs the dark-colored glyph.
render_wordmark "logo-light-eo.svg" "${TABS_DIR}/logo-tab-dark.imageset/logo.png" 86 20
render_wordmark "logo-light-eo.svg" "${TABS_DIR}/logo-tab-dark.imageset/logo_2x.png" 172 40
# logo-tab-light.imageset is used in the DARK theme -> needs the light-colored glyph.
render_wordmark "logo-dark-eo.svg" "${TABS_DIR}/logo-tab-light.imageset/logo_white.png" 86 20
render_wordmark "logo-dark-eo.svg" "${TABS_DIR}/logo-tab-light.imageset/logo_white_2x.png" 172 40

echo ""
echo "Done. Regenerated:"
echo "  ${APPICONSET}/ (10 files)"
echo "  ${TABS_DIR}/logo-tab-dark.imageset/ (2 files)"
echo "  ${TABS_DIR}/logo-tab-light.imageset/ (2 files)"
