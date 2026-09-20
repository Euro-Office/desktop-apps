#!/usr/bin/env bash
# Patches the macOS app's copyright/bundle-name text for the brand being
# built, driven by the same COMPANY_NAME/PRODUCT_NAME values Windows/Linux's
# CI matrix already uses. Run before xcodebuild.
#
# NSHumanReadableCopyright currently reads "Euro-Office was based on
# ONLYOFFICE by Ascensio System SIA" (translated into 43 languages, company
# name left untranslated per the established convention) - replace the
# company-name token with --company-name.
#
# CFBundleName is still the never-fixed "ONLYOFFICE" leftover in the locale
# overrides - replace it with --product-name. Running this unconditionally
# for every brand (including euro-office) finally fixes that bug there too,
# not just for a branded build.
#
# Safe to run repeatedly - a substring that's already been replaced (e.g.
# euro-office's own company name, already "Euro-Office") is a no-op replace.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESOURCES_DIR="${SCRIPT_DIR}/Euro-Office/Resources/Euro-Office-arm"

COMPANY_NAME=""
PRODUCT_NAME=""

usage() {
    cat <<EOF
Usage: $(basename "$0") --company-name <name> --product-name <name>

Required:
  --company-name <name>   Replaces "Euro-Office" in NSHumanReadableCopyright.
  --product-name <name>   Replaces "ONLYOFFICE" in CFBundleName.
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        --company-name) COMPANY_NAME="$2"; shift 2 ;;
        --product-name) PRODUCT_NAME="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "error: unknown argument: $1" 1>&2; usage 1>&2; exit 1 ;;
    esac
done

if [ -z "${COMPANY_NAME}" ] || [ -z "${PRODUCT_NAME}" ]; then
    echo "error: --company-name and --product-name are required." 1>&2
    usage 1>&2
    exit 1
fi

echo "==> Applying branding: company=\"${COMPANY_NAME}\" product=\"${PRODUCT_NAME}\""

INFO_PLIST="${RESOURCES_DIR}/Info.plist"
if [ -f "${INFO_PLIST}" ]; then
    current="$(/usr/libexec/PlistBuddy -c "Print :NSHumanReadableCopyright" "${INFO_PLIST}")"
    replaced="${current//Euro-Office/${COMPANY_NAME}}"
    /usr/libexec/PlistBuddy -c "Set :NSHumanReadableCopyright ${replaced}" "${INFO_PLIST}"
fi

count=0
for strings_file in "${RESOURCES_DIR}"/*.lproj/InfoPlist.strings; do
    [ -f "${strings_file}" ] || continue

    python3 - "${strings_file}" "${COMPANY_NAME}" "${PRODUCT_NAME}" <<'PYEOF'
import re
import sys

path, company_name, product_name = sys.argv[1], sys.argv[2], sys.argv[3]
with open(path, "r", encoding="utf-8") as f:
    text = f.read()

def sub_value(text, key, old, new):
    pattern = re.compile(
        r'("' + re.escape(key) + r'"\s*=\s*")([^"]*)(";)'
    )
    def repl(m):
        return m.group(1) + m.group(2).replace(old, new) + m.group(3)
    return pattern.sub(repl, text)

text = sub_value(text, "NSHumanReadableCopyright", "Euro-Office", company_name)
text = sub_value(text, "CFBundleName", "ONLYOFFICE", product_name)

with open(path, "w", encoding="utf-8") as f:
    f.write(text)
PYEOF

    count=$((count + 1))
done

echo "Patched Info.plist and ${count} InfoPlist.strings files."
