#!/bin/bash

set -exu pipefail

echo "Running strace-hello-world.sh..."

strace -o strace.log -e write target/debug/toy-tcpip-rs
strace -o strace.all.log target/debug/toy-tcpip-rs

set +exu pipefail

echo "Strace script completed successfully!"
