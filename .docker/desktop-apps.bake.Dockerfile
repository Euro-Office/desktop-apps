# ==============================================================================
# MODULE DOCKERFILE
# This file is not meant to be built standalone. It is consumed by the 
# docker-bake.hcl files in the parent monorepos.
#
# REQUIRED CONTEXTS:
# - desktop-sdk: builds/files from this repo
# - core: files from this repo and core-base image
# - core-fonts: files from this repo
# - core-package: the prebuilt core for the core checkout
#   (core/scripts/core_package.py fetch), or an empty directory to build core
# ==============================================================================

#### DESKTOP-APPS
FROM core-base AS desktop-linux

    ARG PRODUCT_VERSION
    ARG BUILD_NUMBER
    ARG CACHE_BUST=3
    ARG TARGETARCH
    ARG BUILD_ROOT
    ARG COMPANY_NAME
    ARG PRODUCT_NAME
    ARG BRANDING_DIR

    # core-base is Rocky Linux 8 on both arches (glibc 2.28, so the result
    # runs on RHEL/Rocky/Alma 8 and newer), see core.bake.Dockerfile. Its
    # desktop variant already has what building Qt needs; these are the
    # app's and CEF's own build dependencies.
    #
    # Keep the toolchain as core-base has it (gcc-toolset-13, clang 21).
    # LLVM's runtime library stays allowed: mesa-dri-drivers needs it
    # (llvm-libs or llvm-compat-libs), and it brings no compiler.
    RUN dnf install -y \
            --exclude="clang*,llvm,llvm-devel,llvm-static,llvm-toolset*,lld*,compiler-rt*,libomp*,gcc-toolset-*" \
            gtk3-devel \
            atk-devel \
            bison \
            libnotify-devel \
            cups-devel \
            wayland-devel \
            dbus-devel \
            libX11-devel \
            libxcb-devel \
            xcb-util-devel \
            xcb-util-cursor-devel \
            xcb-util-keysyms-devel \
            xcb-util-image-devel \
            xcb-util-wm-devel \
            xcb-util-renderutil-devel \
            libXrender-devel \
            libXi-devel \
            libxkbcommon-devel \
            libxkbcommon-x11-devel \
            mesa-libGL-devel \
            mesa-libEGL-devel \
            mesa-libGL \
            mesa-libEGL \
            mesa-dri-drivers \
            mesa-filesystem \
            mesa-libgbm \
            mesa-libglapi \
            alsa-lib-devel \
            pulseaudio-libs-devel \
            nss-devel \
            nspr-devel && \
        dnf clean all

    COPY desktop-sdk /desktop-sdk
    COPY desktop-apps /desktop-apps
    COPY core-fonts /core-fonts
    COPY core /core
    # Docker sees core without its git metadata, so whether the build cache
    # has a package for it is decided outside: an empty context builds core
    COPY --from=core-package / /core-package

    ### Branding
    COPY --from=brand-icons /[d]esktop-apps /desktop-apps
    ###

    COPY --from=desktop-common /index.html /desktop-apps/common/loginpage/deploy/index.html
    COPY --from=desktop-common /editors/webext/noconnect.html /desktop-apps/common/loginpage/deploy/noconnect.html
    #COPY gcc_64 /qt5

    ENV PRODUCT_VERSION=${PRODUCT_VERSION}
    ENV BUILD_NUMBER=${BUILD_NUMBER}
    
    ENV ABOUT_PAGE_APP_NAME="${COMPANY_NAME}"

    # The build tree holds self-built third-party libs (boost, openssl, icu,
    # v8, qt), so it must not be shared across arches/base images: objects
    # from a newer base leak newer glibc/libstdc++ symbols into the build.
    # The id names the base (el8); bump CACHE_BUST for other changes to it.
    RUN --mount=type=cache,target=/build-cache-desktop,id=build-cache-desktop-el8-${TARGETARCH}-${CACHE_BUST} \
        --mount=type=cache,target=/nuget-cache,id=nuget-cache-${CACHE_BUST} \
        --mount=type=cache,target=/ccache,id=ccache \
        export CCACHE_DIR=/ccache && \
        COMPANY_NAME_LOW=$(echo "${COMPANY_NAME}" | tr 'A-Z' 'a-z' | tr ' ' '-') && \
        PRODUCT_NAME_LOW=$(echo "${PRODUCT_NAME}" | tr 'A-Z' 'a-z' | tr -d ' ') && \
        PACKAGE_NAME="${COMPANY_NAME_LOW}-${PRODUCT_NAME_LOW}" && \
        if [ -f /core-package/lib/eo-core-targets.cmake ]; then \
            set -- --core-package /core-package; \
        else \
            set --; \
        fi && \
        /desktop-apps/scripts/build.sh \
            --build-dir /build-cache-desktop --core /core --desktop-sdk /desktop-sdk \
            --tests --ccache "$@" \
            -- -DABOUT_PAGE_APP_NAME="${ABOUT_PAGE_APP_NAME}" \
               -DDESKTOP_FILE_NAME="${PACKAGE_NAME}" && \
        ctest --test-dir /build-cache-desktop/desktop --output-on-failure && \
        cmake --install /build-cache-desktop/desktop && \
        ccache --show-stats && \
        cp -a /build-cache-desktop/desktop/desktopeditors /desktopeditors

    COPY --from=desktop-common / /desktopeditors/

    RUN /desktopeditors/converter/allfontsgen \
        --use-system=1 \
        --input=/desktopeditors/fonts \
        --input=/core-fonts \
        --allfonts=/desktopeditors/converter/AllFonts.js \
        --selection=/desktopeditors/converter/font_selection.bin 
    
    RUN /desktopeditors/converter/allthemesgen \
        --converter-dir=/desktopeditors/converter \
        --src=/desktopeditors/editors/sdkjs/slide/themes \
        --allfonts=/desktopeditors/converter/AllFonts.js \
        --output=/desktopeditors/editors/sdkjs/common/Images

    RUN rm /desktopeditors/converter/allthemesgen && \
        rm /desktopeditors/converter/allfontsgen

    RUN echo 'LD_LIBRARY_PATH=$PWD:$PWD/converter:$LD_LIBRARY_PATH LD_PRELOAD=libcef.so ./DesktopEditors' > /desktopeditors/start_desktop.sh && \
        chmod +x /desktopeditors/start_desktop.sh