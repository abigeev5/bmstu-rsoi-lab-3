# Runtime image shared by all services; SERVICE (build arg) picks which one gets packaged.
#
# The binaries are built by CI on the same Ubuntu 24.04 (`cmake --build --preset release`) and only packaged here:
# building userver inside `docker build` would take 20+ minutes on every run without a compiler cache.
FROM ubuntu:24.04

# Shared libraries the release binaries link against (see `ldd <service>`).
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        libatomic1 \
        libbz2-1.0 \
        libcctz2 \
        libcurl4t64 \
        libfmt9 \
        libgssapi-krb5-2 \
        libicu74 \
        libldap2 \
        liblzma5 \
        libnghttp2-14 \
        libsasl2-2 \
        libssl3t64 \
        libstdc++6 \
        libzstd1 \
        zlib1g \
    && rm -rf /var/lib/apt/lists/*

ARG SERVICE
COPY build/release/${SERVICE}/${SERVICE} /app/service
COPY ${SERVICE}/configs/static_config.yaml /app/static_config.yaml

USER nobody

CMD ["/app/service", "--config", "/app/static_config.yaml"]
