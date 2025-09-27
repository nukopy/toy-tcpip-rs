#!/bin/bash

set -eu

# trap コマンドを使用して、スクリプト終了時や割り込み時(INT, TERM)に cleanup 関数を実行し、生成したバイナリを削除する
# (EXIT はスクリプト終了時、INT は Ctrl+C、TERM は kill などで送られるシグナル)
cleanup() {
  trap - EXIT INT TERM
  echo "cleanup"
  rm -f signal_handling
}

trap cleanup EXIT INT TERM

# build
gcc signal_handling.c -pthread -o signal_handling

# run
./signal_handling

# clean (handled by trap)
