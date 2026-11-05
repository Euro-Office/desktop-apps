#!/usr/bin/env bash
# Renders debian/copyright and fails if upstream additional terms come back
# or the SPDX License fields of the deb and rpm metadata regress.
set -eu
cd "$(dirname "$0")/../.."
make -C package deb/build/debian/copyright
copyright=package/deb/build/debian/copyright
if grep -nE 'Product logo|7\(b\)|7\(e\)' "$copyright" package/common/license/*/*; then
  echo "::error::upstream additional terms are back in the shipped licence metadata"
  exit 1
fi
for field in 'License: AGPL-3.0-only' 'License: CC-BY-SA-4.0' 'License: other'; do
  grep -qx "$field" "$copyright" || { echo "::error::debian/copyright lacks '$field'"; exit 1; }
done
grep -q 'Version 3, 19 November 2007' "$copyright" || { echo "::error::AGPL text not inlined"; exit 1; }
grep -q 'Attribution-ShareAlike 4.0 International' "$copyright" || { echo "::error::CC BY-SA text not inlined"; exit 1; }
grep -qx 'License: AGPL-3.0-only AND CC-BY-SA-4.0' package/rpm/common.spec || { echo "::error::rpm License field regressed"; exit 1; }
