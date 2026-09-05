FROM mcr.microsoft.com/devcontainers/python:1-3.12-bookworm

ENV PIP_DISABLE_PIP_VERSION_CHECK=1 \
    PIP_NO_CACHE_DIR=1

RUN rm -f /etc/apt/sources.list.d/yarn.list \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
        avahi-utils \
        build-essential \
        ca-certificates \
        curl \
        dnsutils \
        git \
        iproute2 \
        iputils-ping \
        make \
        usbutils \
    && rm -rf /var/lib/apt/lists/*

RUN pip install uv==0.11.25
