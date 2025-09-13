#!/bin/bash

set -exuo pipefail

export DEBIAN_FRONTEND=noninteractive

# install packages
apt-get update -y
# apt-get install -y build-essential
# apt-get install -y curl

# install rust non interactive
# ref: https://github.com/rust-lang-deprecated/rustup.sh/issues/83
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

# set cargo path
source $HOME/.cargo/env

# add to bashrc to add Rust toolchains to PATH
echo 'source $HOME/.cargo/env' >> $HOME/.bashrc
