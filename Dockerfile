FROM ubuntu:22.04

# 非対話モードでのパッケージインストールを行うための環境変数の設定
ENV DEBIAN_FRONTEND=noninteractive

# 必要なパッケージをインストール
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    iproute2 \
    iputils-ping \
    netcat-openbsd \
    curl \
    && rm -rf /var/lib/apt/lists/*  # インストール後の不要なファイルを削除

# Rustのインストール
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:${PATH}"

# ソースコードのコピー
COPY . /workspace

# toolchain のインストール　with rust-toolchain.toml
RUN rustup toolchain install --profile default

# ツールのインストール
RUN cargo install cargo-binstall
RUN cargo binstall rustowl
