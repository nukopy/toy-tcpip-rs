#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * シンプルなデモ: 標準シグナル（SIGUSR2）とリアルタイムシグナル（SIGRTMIN）で
 * 保留（pending）の挙動がどう違うかを確認する。
 *
 * - 標準シグナル (SIGUSR1/SIGUSR2 など): 同じ種類がキューに 1 個しか保留されない。
 *   -> ブロック中に何度送っても 1 回分に折りたたまれる。
 *
 * - リアルタイムシグナル (SIGRTMIN など): 各シグナルが個別にキューへ積まれる。
 *   -> ブロック中に複数回送れば、その回数分だけハンドラが連続で呼ばれる。
 */

// --------------------------------------------------
// 標準シグナルの保留
// --------------------------------------------------

static volatile sig_atomic_t standard_handler_count;

static void on_sigusr2(int signo)
{
  standard_handler_count++;

  // シグナル安全ではないけどデバッグのため printf を利用
  printf("[standard handler] got SIGUSR2 (%d): count=%d\n", signo,
         standard_handler_count);
}

static void pending_standard_signal(void)
{
  // シグナルアクションの初期化
  struct sigaction sa = {};

  // シグナルアクションインスタンスにシグナルハンドラを設定
  sa.sa_handler = on_sigusr2;

  // シグナルアクションのシグナルマスクを 0 初期化
  /* 注意: sa.sa_mask はプロセスのシグナルマスクとは異なる
   * sa.sa_mask が効くのは「ハンドラを実行している最中に追加でブロックするシグナル」の指定だけで、
   * 平常時にそのシグナルをブロックして保留させることはできない。
   * 今回のデモではハンドラが走る前に SIGUSR2 / SIGRTMIN を一時的にブロックしてキューに貯めたいので、
   * sigprocmask（またはスレッド単位なら pthread_sigmask）のようにプロセス／スレッドのシグナルマスクを直接変更する API が必須。
   * そのため、sigaction の sa_mask に置き換える形ではこの挙動を再現できない。
   */
  sigemptyset(&sa.sa_mask);

  // sigaction.sa_flags: シグナルハンドラの動作を制御するためのビットフラグの設定
  /* ここは参考程度に。
   * `sa_flags` は `struct sigaction` のメンバで、シグナルハンドラの動作を制御するためのビットフラグをセットする場所である。
   * `SA_RESTART` で割り込み後にシステムコールを再開したり、`SA_SIGINFO` で拡張情報を受け取ったり、
   * `SA_NOCLDWAIT` や `SA_NODEFER` など、用途に応じた定義済みマクロを OR で組み合わせて指定する。
   * 例えば `sig.sa_flags = SA_RESTART | SA_SIGINFO;` のように書くことで、対象シグナルの扱い方を細かく調整できる。
   */
  sa.sa_flags = 0;

  // シグナルアクションの登録
  if (sigaction(SIGUSR2, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  // プロセスへシグナルマスクの設定
  // - シグナル集合の定義
  sigset_t mask;
  sigset_t oldset;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR2);

  // - シグナルマスクの設定（シグナルをブロック）
  if (sigprocmask(SIG_BLOCK, &mask, &oldset) == -1) {
    perror("sigprocmask(SIG_BLOCK)");
    exit(EXIT_FAILURE);
  }
  printf("signal SIGUSR2 masked\n");

  /* SIGUSR2 を 3 回連続で raise() するが、保留できるのは 1 つだけ */
  printf("raising SIGUSR2 three times...\n");
  for (int i = 0; i < 3; i++) {
    printf("[standard] raise(SIGUSR2) #%d\n", i + 1);
    /* raise 関数について
     * raise(signo) は「今動いている自プロセスに対してシグナル signo を 1 回送る」ための標準 C ライブラリ関数。
     * 内部的には kill(getpid(), signo) とほぼ同じことをしており、プロセス自身が自分にシグナルを送るだけなので、
     * SIGKILL のようにキャッチや無視ができないシグナルを送っても、結局そのシグナルの既定動作（SIGKILL なら即終了）がそのまま実行される。
     * ハンドラで挙動を試したい標準シグナルやリアルタイムシグナルを、自前のテストプログラムの中から簡単に発火させるのに便利な関数。
     */
    if (raise(SIGUSR2) != 0) {
      perror("raise");
      exit(EXIT_FAILURE);
    }
  }

  // unblock
  printf("[starndard] unblock after 3 seconds...\n");
  for (int i = 3; i >= 1; i--) {
    printf("[standard] countdown: %d\n", i);
    sleep(1);
  }
  if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
    perror("sigprocmask(SIG_SETMASK)");
    exit(EXIT_FAILURE);
  }
  printf("[starndard] unblocked!\n");

  // スリープ
  /*
  `unblock_signal(&oldset)` を呼んだ直後は、ブロックしていたシグナルが解放されるだけで、
  ハンドラがその場で同期的に実行される保証はない。スケジューラがいつハンドラを走らせるかは未定義なので、
  すぐに後続の `printf` などを実行すると、ハンドラがまだ走っておらずカウンタが増えていないまま結果を表示してしまう、
  最悪そのまま関数やプロセスが終了してハンドラが実行されない、といった状況が起こり得る。

  `sleep(1)` を挟んでいるのは、あえて少し待つことで「ハンドラが走る時間を作り、デモの出力を安定させている」ため。
  */
  printf("sleep 1 sec to trigger signal handler for SIGUSR2\n");
  sleep(1);

  // シグナルハンドラーが何回呼ばれたかを出力
  printf("[standard] handler count=%d (expected: 1)\n",
         standard_handler_count);
}

// --------------------------------------------------
// リアルタイムシグナルの保留
// --------------------------------------------------

static volatile sig_atomic_t realtime_handler_count;

static void on_sigrtmin(int signo, siginfo_t *info, void *ucontext)
{
  (void)info;
  (void)ucontext;
  realtime_handler_count++;

  // シグナル安全ではないけどデバッグのため printf を利用
  printf("[realtime handler] got signo=%d (count=%d)\n", signo,
         realtime_handler_count);
}

static void pending_realtime_signal(void)
{
  // シグナルアクションの初期化
  int signo = SIGRTMIN;
  struct sigaction sa = {};

  // シグナルアクションインスタンスにシグナルハンドラを設定
  sa.sa_sigaction = on_sigrtmin;

  // シグナルアクションのシグナルマスクを 0 初期化
  sigemptyset(&sa.sa_mask);

  // sigaction.sa_flags: シグナルハンドラの動作を制御するためのビットフラグの設定
  sa.sa_flags = SA_SIGINFO;
  if (sigaction(signo, &sa, NULL) == -1) {
    perror("sigaction (SIGRTMIN)");
    exit(EXIT_FAILURE);
  }

  // プロセスへシグナルマスクの設定
  sigset_t mask;
  sigset_t oldset;
  sigemptyset(&mask);
  sigaddset(&mask, signo);
  if (sigprocmask(SIG_BLOCK, &mask, &oldset) == -1) {
    perror("sigprocmask(SIG_BLOCK)");
    exit(EXIT_FAILURE);
  }
  printf("signal SIG_BLOCK masked\n");

  /* SIGRTMIN を 3 回連続で raise() すると、3 回ともキューに積まれる */
  printf("raising SIGRTMIN three times...\n");
  for (int i = 0; i < 3; i++) {
    printf("[realtime] raise(SIGRTMIN) #%d\n", i + 1);
    if (raise(signo) != 0) {
      perror("raise (SIGRTMIN)");
      exit(EXIT_FAILURE);
    }
  }

  // unblock
  printf("[realtime] unblock after 3 seconds...\n");
  for (int i = 3; i >= 1; i--) {
    printf("[realtime] countdown: %d\n", i);
    sleep(1);
  }
  if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
    perror("sigprocmask(SIG_SETMASK)");
    exit(EXIT_FAILURE);
  }
  printf("[realtime] unblocked!\n");

  // スリープ
  printf("sleep 1 sec to trigger signal handler for SIGRTMIN\n");
  sleep(1);

  // シグナルハンドラーが何回呼ばれたかを出力
  printf("[realtime] handler count=%d (expected: 3)\n",
         realtime_handler_count);
}

int main(void)
{
  printf("\n==== pending_standard_signal (SIGUSR2) ====\n標準シグナルでは同じ種類のシグナルは 1 個しか保留できない\n");
  pending_standard_signal();

  printf("\n==== pending_realtime_signal (SIGRTMIN) ====\nリアルタイムシグナルでは同じ種類を 1 つ以上保留できる\n");
  pending_realtime_signal();

  return EXIT_SUCCESS;
}
