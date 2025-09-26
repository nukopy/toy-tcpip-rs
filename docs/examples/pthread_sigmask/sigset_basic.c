#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>

/*
 * sigset_basic.c
 * --------------
 * 目的: sigset_t の操作だけを目で追えるようにするシンプルなデモ。
 *       シグナルハンドラや sigprocmask は使わず、集合操作の結果を表示する。
 *
 * 使用する主な API:
 *   - sigemptyset(set)  : 集合を空に初期化
 *   - sigfillset(set)   : すべてのシグナルを集合に追加
 *   - sigaddset(set,x)  : x を集合に追加
 *   - sigdelset(set,x)  : x を集合から削除
 *   - sigismember(set,x): x が集合に入っているか調べる
 */

static void dump_sigset(const sigset_t *set) {
  const int interesting[] = {SIGHUP, SIGINT, SIGTERM};
  const char *names[] = {"SIGHUP", "SIGINT", "SIGTERM"};

  printf("signal set members        :");
  for (size_t i = 0; i < sizeof(interesting) / sizeof(interesting[0]); i++) {
    if (sigismember(set, interesting[i])) {
      printf(" %s (%d),", names[i], interesting[i]);
    }
  }
  printf("\n");

  const unsigned long *raw = (const unsigned long *)set;
  unsigned long value = raw[0]; /* 先頭要素のみ確認する */
  printf("raw signal set (__val[0]) : 0b");
  for (int bit = 15; bit >= 0; bit--) {
    unsigned long mask = 1UL << bit;
    printf("%lu", (value & mask) ? 1UL : 0UL);
    if (bit % 8 == 0 && bit != 0) {
      printf("_"); /* 4 ビットごとに区切りを入れる */
    }
  }
  printf("\n\n");
}

int main(void) {
  sigset_t set;

  printf("[step1] sigemptyset: 空集合を作成\n");
  // 集合を空にする
  sigemptyset(&set);
  dump_sigset(&set);

  printf("[step2] sigfillset: 全てのシグナルを集合に追加\n");
  sigfillset(&set);
  dump_sigset(&set);

  printf("[step3] sigemptyset: 集合を空集合に戻す\n");
  sigemptyset(&set);
  dump_sigset(&set);

  printf("[step4] sigaddset: 集合に SIGINT を追加\n");
  sigaddset(&set, SIGINT);
  dump_sigset(&set);

  printf("[step5] sigaddset: 集合に SIGTERM を追加\n");
  sigaddset(&set, SIGTERM);
  dump_sigset(&set);

  printf("[step6] sigismember: SIGINT, SIGHUP, SIGTERM が集合に含まれるか判定\n");
  int sigint_contains = sigismember(&set, SIGINT);
  printf("SIGINT   : %s\n", sigint_contains ? "in" : "not in");
  int sighup_contains = sigismember(&set, SIGHUP);
  printf("SIGHUP   : %s\n", sighup_contains ? "in" : "not in");
  int sigterm_contains = sigismember(&set, SIGTERM);
  printf("SIGHTERM : %s\n", sigterm_contains ? "in" : "not in");
  printf("\n");

  printf("[step7] sigaddset: 集合に SIGHUP を追加\n");
  sigaddset(&set, SIGHUP);
  dump_sigset(&set);

  printf("[step8] sigdelset: SIGINT を集合から削除\n");
  sigdelset(&set, SIGINT);
  dump_sigset(&set);

  printf("[step9] sigemptyset: 全てのシグナルを集合から削除\n");
  sigemptyset(&set);
  dump_sigset(&set);

  printf("DONE!\n");

  return 0;
}
