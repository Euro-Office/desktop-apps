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

    COPY desktop-apps/common app

    RUN cd app/loginpage/build && \
        npm install && \
        grunt