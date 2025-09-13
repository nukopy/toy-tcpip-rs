#!/bin/bash

set -exu pipefail

export DEBIAN_FRONTEND=noninteractive

echo "Bootstrapping VM for toy-tcpip-rs..."
echo "Run script as non-root user: whoami=$(whoami)"

# install packages
sudo apt-get update -y && sudo apt-get install -y gcc

# install rust non interactive
# ref: https://github.com/rust-lang-deprecated/rustup.sh/issues/83
curl --proto "=https" --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

# set cargo path
source $HOME/.cargo/env
echo "cargo version: $(cargo --version)"

# add to bashrc to add Rust toolchains to PATH
echo "source $HOME/.cargo/env" >> $HOME/.bashrc

set +exu pipefail

echo "Bootstrap script completed successfully!"
