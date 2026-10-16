# ==============================================================================
# MODULE DOCKERFILE
# This file is not meant to be built standalone. It is consumed by the 
# docker-bake.hcl files in the parent monorepos.
# ==============================================================================

#### BASE ####
FROM ubuntu:24.04 AS web-base
    RUN apt-get update && \
        apt-get install -y ca-certificates curl gnupg openjdk-21-jdk wget zip brotli bzip2 && \
        curl -fsSL https://deb.nodesource.com/setup_20.x | bash - && \
        apt-get install -y nodejs && \
        npm install -g @yao-pkg/pkg grunt-cli && \
        rm -rf /var/lib/apt/lists/*

#### DESKTOP-JS
FROM web-base AS desktop-js
    ARG COMPANY_NAME
    ARG BUILD_COMMITS
    ARG BUILD_DATE

    COPY desktop-apps/common app
    COPY web-apps/build/about-html.mjs /web-apps/build/
    COPY web-apps/theme /web-apps/theme

    ENV COMPANY_NAME=${COMPANY_NAME}
    ENV BUILD_COMMITS=${BUILD_COMMITS}
    ENV BUILD_DATE=${BUILD_DATE}
    ENV ABOUT_HTML_SCRIPT=/web-apps/build/about-html.mjs
    RUN cd app/loginpage/build && \
        npm install && \
        grunt