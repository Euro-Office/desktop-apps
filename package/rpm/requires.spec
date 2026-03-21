%if "%{_package_edition}" == "full"
Requires: curl, libX11, xdg-utils, dejavu-sans-fonts, dejavu-sans-mono-fonts, dejavu-serif-fonts, liberation-mono-fonts, liberation-sans-fonts, liberation-serif-fonts
%else
Requires: curl, libX11, libXScrnSaver, atk, gtk3, libstdc++ >= 4.8.0, boost-filesystem, xcb-util-renderutil, xcb-util-image, xcb-util-wm, libxcb, xcb-util-keysyms, xdg-utils, libxkbcommon-x11, libnotify, dejavu-sans-fonts, dejavu-sans-mono-fonts, dejavu-serif-fonts, liberation-mono-fonts, liberation-narrow-fonts, liberation-sans-fonts, liberation-serif-fonts
%endif
