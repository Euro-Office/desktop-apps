Source: M4_PACKAGE_NAME
Section: editors
Priority: optional
Maintainer: M4_PUBLISHER_NAME <M4_SUPPORT_MAIL>
Build-Depends: debhelper (>= 8.0.0)

Package: M4_PACKAGE_NAME
#Standards-Version: 3.9.4
Architecture: M4_DEB_ARCH
Pre-Depends: dpkg (>= 1.14.0)
Provides: M4_PACKAGE_OPENSOURCE
Depends:
ifelse(M4_PACKAGE_EDITION, full,
` x11-common,
  curl | wget,
  fonts-dejavu | ttf-dejavu'
,
` x11-common,
  libasound2,
  curl | wget,
  desktop-file-utils,
  gstreamer1.0-libav,
  gstreamer1.0-plugins-ugly,
  libxss1,
  libatk1.0-0,
  libgtk-3-0,
  libcairo2,
  libstdc++6 (>=4.8),
  libxkbcommon-x11-0,
  libnotify4,
  fonts-dejavu | ttf-dejavu,
  fonts-liberation,
  fonts-crosextra-carlito,
  xdg-utils')
Recommends: ttf-mscorefonts-installer, fonts-takao-gothic 
ifelse(M4_COMPANY_NAME, ONLYOFFICE,Suggests: M4_PACKAGE_NAME-help
,)dnl
Conflicts: ifelse(M4_PACKAGE_EDITION, commercial, M4_PACKAGE_OPENSOURCE, M4_PACKAGE_COMMERCIAL)
Description: Desktop editors for text docs, spreadsheets, presentations, PDFs, and PDF forms.
 Open-source office suite pack that comprises all the tools you need to
 work offline with documents, spreadsheets, presentations, PDFs, and PDF forms.

ifelse(M4_COMPANY_NAME, ONLYOFFICE,
Package: M4_PACKAGE_NAME-help
Architecture: all
Pre-Depends: dpkg (>= 1.14.0)
Depends: M4_PACKAGE_OPENSOURCE
Enhances: M4_PACKAGE_OPENSOURCE
Description: offline help for M4_COMPANY_NAME M4_PRODUCT_NAME
 This package contains offline help files.,)
