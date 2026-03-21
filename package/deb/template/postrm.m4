#!/bin/sh

set -e

case "$1" in
  purge)
    rm -fr /home/*/.local/share/M4_DESKTOPEDITORS_PREFIX
    rm -fr /home/*/.config/M4_COMPANY_NAME_LOW/DesktopEditors.conf
  ;;

  remove|upgrade)
    rm -f /usr/share/mime/packages/onlyoffice-docxf.xml
    rm -f /usr/share/mime/packages/onlyoffice-oform.xml
  ;;

  failed-upgrade|abort-install|abort-upgrade|disappear)
  ;;

  *)
    echo "postrm called with unknown argument \`$1'" >&2
    exit 1
  ;;
esac

#DEBHELPER#

exit 0
