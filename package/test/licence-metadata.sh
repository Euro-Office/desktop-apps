#!/usr/bin/env bash
# Renders debian/copyright and fails if upstream additional terms come back,
# the Section 7(a) warranty term goes missing or the SPDX License fields of
# the deb and rpm metadata regress, and checks that the Windows copyright
# strings name Euro-Office contributors and that the deb and rpm ship a
# NOTICE with the ONLYOFFICE origin notice.
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
tr -s ' \n' '  ' < "$copyright" | grep -qF 'In accordance with Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect that Ascensio System SIA expressly excludes the warranty of non-infringement of any third-party rights.' \
  || { echo "::error::debian/copyright lacks the Section 7(a) warranty term"; exit 1; }
grep -q 'Version 3, 19 November 2007' "$copyright" || { echo "::error::AGPL text not inlined"; exit 1; }
grep -q 'Attribution-ShareAlike 4.0 International' "$copyright" || { echo "::error::CC BY-SA text not inlined"; exit 1; }
grep -qx 'License: AGPL-3.0-only AND CC-BY-SA-4.0' package/rpm/common.spec || { echo "::error::rpm License field regressed"; exit 1; }
for f in package/inno/defines.iss win-linux/src/version.h win-linux/extras/update-daemon/src/version.h win-linux/extras/projicons/src/version.h; do
  grep -q 'Ascensio System SIA, Euro-Office contributors' "$f" || { echo "::error::$f copyright string lacks Euro-Office contributors"; exit 1; }
done
# Brands of the DesktopEditors build matrix (COMPANY_NAME = PRODUCT_NAME there).
for brand in 'Euro-Office' 'Nextcloud Office'; do
  vars=("COMPANY_NAME=$brand" "PRODUCT_NAME=$brand")
  notice=$(make -C package -n -W NOTICE.m4 build "${vars[@]}" | sed -n 's/.* > \(build\/.*\/NOTICE\)$/\1/p')
  [ -n "$notice" ] || { echo "::error::NOTICE is not part of the $brand package layout"; exit 1; }
  make -B -C package "$notice" "${vars[@]}"
  tr -s ' \n' '  ' < "package/$notice" | grep -qF "$brand is based on the original ONLYOFFICE software developed by Ascensio System SIA. $brand is not affiliated with" \
    || { echo "::error::$brand NOTICE lacks the ONLYOFFICE origin notice"; exit 1; }
done
