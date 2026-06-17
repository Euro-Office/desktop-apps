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
    ARG CACHE_BUST=1
    ARG BUILD_ROOT
    ARG COMPANY_NAME
    ARG PRODUCT_NAME
    ARG BRANDING_DIR

    RUN apt-get -y update && \
        apt-get -y upgrade && \ 
        apt-get -y install \
                    libgtk-3-dev \
                    libatk1.0-dev \
                    libxkbcommon-x11-dev \
                    python3-venv \
                    bison \
                    libnotify-dev \
                    libcups2-dev \
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
                    libnspr4-dev

    COPY desktop-sdk /desktop-sdk
    COPY desktop-apps /desktop-apps
    COPY core-fonts /core-fonts

    ### Branding
    COPY ${BRANDING_DIR}/desktop-apps /desktop-apps
    ###

    COPY --from=desktop-common /index.html /desktop-apps/common/loginpage/deploy/index.html
    COPY --from=desktop-common /editors/webext/noconnect.html /desktop-apps/common/loginpage/deploy/noconnect.html
    #COPY gcc_64 /qt5

    ENV PRODUCT_VERSION=${PRODUCT_VERSION}
    ENV BUILD_NUMBER=${BUILD_NUMBER}
    
    ENV ABOUT_PAGE_APP_NAME="${COMPANY_NAME} ${PRODUCT_NAME}"

    RUN --mount=type=cache,target=/build-cache-desktop,id=build-cache-desktop-${CACHE_BUST} \
        --mount=type=cache,target=/nuget-cache,id=nuget-cache-${CACHE_BUST} \
        --mount=type=bind,from=third-party,source=/third_party,target=/third_party_src \
        --mount=type=secret,id=nextcloud_user \
        --mount=type=secret,id=nextcloud_pass \
        export NEXTCLOUD_USER="$(cat /run/secrets/nextcloud_user)" && \
        export NEXTCLOUD_PASS="$(cat /run/secrets/nextcloud_pass)" && \
        cp -a /third_party_src/. /build-cache-desktop/third_party && \
        cd /build-cache-desktop && \
        cmake -GNinja \
              -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
              -DVCPKG_MANIFEST_MODE=ON \
              -DVCPKG_MANIFEST_DIR="/core" \
              -DABOUT_PAGE_APP_NAME="${ABOUT_PAGE_APP_NAME}" \
              /desktop-apps/win-linux/ && \
        cmake --build . && \
        cmake --install . && \
        cp -a desktopeditors /desktopeditors

    COPY --from=desktop-common / /desktopeditors/


#### ALLFONTSGEN / ALLTHEMESGEN ####
FROM core-base AS allgen-builder
    ARG NUGET_SOURCE_PATH
    ARG TARGETARCH

    COPY core-fonts /core-fonts

    RUN --mount=type=cache,target=/build-cache-allgen-1 \
        --mount=type=bind,source=${NUGET_SOURCE_PATH},target=/nuget-cache,rw \
        --mount=type=secret,id=nextcloud_user \
        --mount=type=secret,id=nextcloud_pass \
        export NEXTCLOUD_USER="$(cat /run/secrets/nextcloud_user)" && \
        export NEXTCLOUD_PASS="$(cat /run/secrets/nextcloud_pass)" && \
        mkdir -p ${BUILD_ROOT} /tmp/allgen && \
        printf '%s\n' \
               'cmake_minimum_required(VERSION 3.16)' \
               'project(allgen)' \
               'set(CORE_ROOT_DIR "/core")' \
               'include(${CORE_ROOT_DIR}/common.cmake)' \
               'add_subdirectory(${CORE_ROOT_DIR}/DesktopEditor/AllFontsGen AllFontsGen)' \
               'add_subdirectory(${CORE_ROOT_DIR}/DesktopEditor/allthemesgen allthemesgen)' \
               'add_subdirectory(${CORE_ROOT_DIR}/X2tConverter/build/cmake x2t )' \
               > /tmp/allgen/CMakeLists.txt && \
        cd /build-cache-allgen-1 && \
        cmake -GNinja \
              -DVCPKG_MANIFEST_MODE=ON \
              -DVCPKG_MANIFEST_DIR=/core \
              -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
              -DEO_CORE_OUTPUT_DIR=/build-cache-allgen-1/package \
              -DEO_CORE_TOOLS_DIR=/build-cache-allgen-1/package \
              /tmp/allgen && \
        cmake --build . && \
        cp -r package/* ${BUILD_ROOT}