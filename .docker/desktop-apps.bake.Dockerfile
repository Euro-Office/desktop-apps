# ==============================================================================
# MODULE DOCKERFILE
# This file is not meant to be built standalone. It is consumed by the 
# docker-bake.hcl files in the parent monorepos.
#
# REQUIRED CONTEXTS:
# - desktop-sdk: builds/files from this repo
# - core: files from this repo and core-base image
# - core-fonts: files from this repo
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

    # core-base is Rocky Linux 9 on amd64 (glibc 2.34, so the result runs on
    # RHEL/Rocky/Alma 9) and Ubuntu 24.04 on arm64, see core.bake.Dockerfile.
    #
    # On Rocky, current mesa needs LLVM 21, and dnf would replace the pinned
    # clang 13 (V8) with clang 21 + gcc-toolset-15. Its libstdc++ headers emit
    # GCC 13+ symbols (std::ios_base_library_init) that el9's libstdc++ lacks.
    # Keep the toolchain fixed and take mesa from the Rocky 9.0 vault (built
    # against LLVM 13), which is only needed for GL/EGL headers at build time.
    ARG MESA_EL9_VERSION=21.3.4-2.el9
    RUN if command -v dnf >/dev/null; then \
            dnf install -y \
                    --enablerepo=rocky90-appstream \
                    --setopt=rocky90-appstream.module_hotfixes=1 \
                    --exclude="clang*,llvm*,lld*,compiler-rt*,libomp*,gcc-toolset-*" \
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
                    mesa-libGL-devel-${MESA_EL9_VERSION} \
                    mesa-libEGL-devel-${MESA_EL9_VERSION} \
                    mesa-libGL-${MESA_EL9_VERSION} \
                    mesa-libEGL-${MESA_EL9_VERSION} \
                    mesa-dri-drivers-${MESA_EL9_VERSION} \
                    mesa-filesystem-${MESA_EL9_VERSION} \
                    mesa-libgbm-${MESA_EL9_VERSION} \
                    mesa-libglapi-${MESA_EL9_VERSION} \
                    alsa-lib-devel \
                    pulseaudio-libs-devel \
                    nss-devel \
                    nspr-devel && \
            dnf clean all; \
        else \
            apt-get -y update && \
            apt-get -y upgrade && \
            apt-get -y install \
                        libgtk-3-dev \
                        libatk1.0-dev \
                        libxkbcommon-x11-dev \
                        python3-venv \
                        python3-pip \
                        bison \
                        libnotify-dev \
                        libcups2-dev \
                        libwayland-dev \
                        libdbus-1-dev \
                        libxcb-util0-dev \
                        libxcb-xkb-dev \
                        libxcb-cursor-dev \
                        libxcb-xinput-dev \
                        build-essential \
                        ninja-build \
                        pkg-config \
                        libx11-dev \
                        libx11-xcb-dev \
                        libxcb1-dev \
                        libxcb-render0-dev \
                        libxcb-shape0-dev \
                        libxcb-xfixes0-dev \
                        libxcb-randr0-dev \
                        libxcb-keysyms1-dev \
                        libxcb-image0-dev \
                        libxcb-icccm4-dev \
                        libxcb-sync-dev \
                        libxcb-xinerama0-dev \
                        libxcb-util-dev \
                        libxrender-dev \
                        libxi-dev \
                        libxkbcommon-dev \
                        libxkbcommon-x11-dev \
                        libgl1-mesa-dev \
                        libegl1-mesa-dev \
                        libasound2-dev \
                        libpulse-dev \
                        libnss3-dev \
                        libnspr4-dev && \
            rm -rf /var/lib/apt/lists/*; \
        fi && \
        ${CORE_PYTHON} -m venv /opt/venv && \
        /opt/venv/bin/pip install --no-cache-dir aqtinstall

    ENV PATH="/opt/venv/bin:$PATH"

    COPY desktop-sdk /desktop-sdk
    COPY desktop-apps /desktop-apps
    COPY core-fonts /core-fonts
    COPY core /core

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
    # v8), so it must not be shared across arches/base images: objects from the
    # Ubuntu base leak newer glibc/libstdc++ symbols into the Rocky 9 build.
    # Bump CACHE_BUST whenever the base image of an arch changes.
    RUN --mount=type=cache,target=/build-cache-desktop,id=build-cache-desktop-${TARGETARCH}-${CACHE_BUST} \
        --mount=type=cache,target=/nuget-cache,id=nuget-cache-${CACHE_BUST} \
        --mount=type=cache,target=/ccache,id=ccache \
        --mount=type=secret,id=nextcloud_user \
        --mount=type=secret,id=nextcloud_pass \
        export NEXTCLOUD_USER="$(cat /run/secrets/nextcloud_user)" && \
        export NEXTCLOUD_PASS="$(cat /run/secrets/nextcloud_pass)" && \
        export CCACHE_DIR=/ccache && \
        cd /build-cache-desktop && \
        cmake -GNinja \
            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
            -DVCPKG_MANIFEST_MODE=ON \
            -DVCPKG_MANIFEST_DIR="/core" \
            -DVCPKG_MANIFEST_FEATURES="desktop-editors" \
            -DPYTHON_BIN=/opt/venv/bin/python \
            -DABOUT_PAGE_APP_NAME="${ABOUT_PAGE_APP_NAME}" \
            /desktop-apps/win-linux/ && \
        cmake --build . && \
        cmake --install . && \
        ccache --show-stats && \
        cp -a desktopeditors /desktopeditors

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