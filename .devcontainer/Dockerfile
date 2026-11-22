FROM ubuntu:26.04

# Install common utilities
RUN apt-get update && apt-get install -y \
    python-is-python3 \
    python3 \
    python3-pip \
    pipx \
    bash \
    curl \
    findutils \
    git \
    unzip \
    npm \
    rustup \
    build-essential \
    openssl \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Headless Chromium for the Playwright e2e suite. Ubuntu's own chromium package
# is a snap wrapper that cannot run inside a container (and the xtradeb PPA has
# no 26.04 builds), so install the browser revision that matches the project's
# Playwright, together with the system libraries it links against. Keep the
# pinned version in sync with package-lock.json's @playwright/test; the shared
# browsers directory stays writable so a newer Playwright can stage a newer
# revision at test time without a rebuild.
ENV PLAYWRIGHT_BROWSERS_PATH=/usr/local/share/ms-playwright
RUN npm i -g playwright@1.63.0 \
    && DEBIAN_FRONTEND=noninteractive playwright install --with-deps chromium \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /usr/local/share/ms-playwright \
    && chown -R ubuntu:ubuntu /usr/local/share/ms-playwright

USER ubuntu

# Install PlatformIO
RUN pipx install platformio
RUN pipx ensurepath

# Install opencode
RUN npm config set prefix '~/.local/'
RUN npm i -g opencode-ai@latest
RUN mkdir -p ~/.local/share/opencode

# Install ccusage
RUN npm i -g ccusage@latest

# Update Rust
RUN rustup default stable
RUN rustup update

# Install tokensave
RUN cargo install tokensave

# Install rtk
RUN cargo install --git https://github.com/rtk-ai/rtk rtk

# Install opencode hooks
ENV PATH="${PATH}:/home/ubuntu/.cargo/bin/"
RUN tokensave install --agent opencode
RUN rtk init --global --opencode

# Set working directory
WORKDIR /workspace

# Initialize tokensave & rtk
# RUN tokensave init --git-hook
RUN tokensave disable-upload-counter
RUN rtk telemetry disable

# Default shell
CMD ["bash"]
