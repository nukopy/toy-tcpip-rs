#!/bin/bash

set -eu

# build
gcc sigset_basic.c -o sigset_basic

# run
./sigset_basic

# clean
rm sigset_basic
