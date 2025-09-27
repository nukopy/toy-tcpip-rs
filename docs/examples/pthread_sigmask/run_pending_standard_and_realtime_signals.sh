#!/bin/bash

set -eu

FILENAME=pending_standard_and_realtime_signals

cleanup() {
  trap - EXIT INT TERM
  rm -f $FILENAME
}

trap cleanup EXIT INT TERM

# build
gcc $FILENAME.c -o $FILENAME

# run
./$FILENAME

# clean (handled by trap)
