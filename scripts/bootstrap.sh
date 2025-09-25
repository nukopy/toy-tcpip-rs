#!/bin/bash

set -exu -o pipefail

export DEBIAN_FRONTEND=noninteractive

echo "Bootstrapping VM for toy-tcpip-rs..."
echo "Run script as non-root user: whoami=$(whoami)"

# --------------------------------------------------
# install general packages
# ref: KLab Expert Camp 6 day 1: https://docs.google.com/presentation/d/1ID6ggxASfc_1bWiJfDy1IFKIwzxvfYy8rUBrWYTRFj8/edit?slide=id.gd328c3072b_0_1025#slide=id.gd328c3072b_0_1025
# --------------------------------------------------

# development tools
sudo apt-get update -y && sudo apt-get install -y \
  build-essential \
  gdb \
  clangd \
  git \
  strace \
  tree \
  clang-format

# network tools
# sudo apt-get update -y && sudo apt-get install -y \
#   iproute2 \
#   iputils-ping \
#   netcat-openbsd

# --------------------------------------------------
# setup Rust
# --------------------------------------------------

# ref: Install Rust non interactively: https://github.com/rust-lang-deprecated/rustup.sh/issues/83
curl --proto "=https" --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

# set cargo path
source $HOME/.cargo/env
echo "cargo version: $(cargo --version)"

# add to bashrc to add Rust toolchains to PATH
echo "source $HOME/.cargo/env" >> $HOME/.bashrc

# --------------------------------------------------
# setup git
# --------------------------------------------------

USERNAME="nukopy"
EMAIL="nukopy@gmail.com"
git config --global user.name "$USERNAME"
git config --global user.email "$EMAIL"

set +exu +o pipefail

echo "Bootstrap script completed successfully!"
