# ベースイメージとして Ubuntu 20.04 を指定
FROM ubuntu:20.04

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

# コンテナのデフォルトコマンドを bash に設定
CMD ["bash"]
