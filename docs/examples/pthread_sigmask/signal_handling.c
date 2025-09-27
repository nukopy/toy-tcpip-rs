#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* サンプルコードの目的: マルチスレッド環境でのシグナルの扱いを理解する
 *
 * ## シグナルを扱うための関数の説明
 *
 * ### シグナル関連
 *
 * - sigprocmask
 *   - プロセスが単一スレッドの段階でシグナルマスクを設定する伝統的な API
 *   - シグナルマスクはスレッド単位で設定されるものである。
 *   - シグナルマスクをプロセス / スレッドに設定すると、そのプロセス / スレッドはカーネルからのシグナルによる通知を止めることができる。
 *     通知を停止しているときに届いたシグナルは「未処理のシグナル」としてキューに残り、
 *     マスク解除や `sigwait` などで受け取るタイミングまでスレッドに渡されない。
 *   - sigprocmask 関数は、メインスレッド以外のスレッド（ワーカースレッド）でも呼べてしまうが、
 *     複数スレッドが起動しているマルチスレッド環境での実行では未定義動作とされている（POSIX 的に未定義）。
 *   - pthread_create などで新しくスレッドを生成した場合、子スレッドには生成元のスレッドのシグナルマスクが継承される。
 *     今回の場合、メインスレッドで sigprocmask 関数を使って SIGINT や SIGUSR1 をマスクすると、ワーカースレッドにはそのシグナルマスクが継承される。
 *   - 主な用途としては、スレッド生成前に sigprocmask でメインスレッドのシグナルマスクを設定し、子スレッドへ継承させること
 * - pthread_sigmask
 *   - スレッド単位でシグナルマスクを変更する (マルチスレッド環境向け API)
 *   - 先述したように、sigprocmask 関数はマルチスレッド環境では未定義動作を起こす。
 * - sigwait
 *   - 事前にマスクしておいたシグナルが届くまでブロックし、届いたシグナル番号を安全に受け取る API
 *   - シグナルハンドラ内で処理をする代わりに、同期ポイントでシグナルを扱いたいときに使う
 *   - 多くの場合 `pthread_sigmask` で対象シグナルをブロックした状態で使うのが定石
 *
 * #### sigwait 補足
 *
 * `sigwait` を呼んだスレッドは、カーネル内で「そのシグナルが届くまでスリープ」状態になる。
 *
 * タスクスケジューラから見ると以下のような扱いとなる：
 *
 * - スレッドはシステムコール経由でカーネルに入り、対象シグナルを待つキューに登録される
 * - 待ち条件が満たされる（指定したシグナルがブロック解除されて配信可能になる）まで、スケジューラのランキューから外されるので CPU を消費しない
 * - シグナルが届くと、そのスレッドがウェイクアップされ、ランキューへ戻されて再び実行可能になる
 *
 * つまり「シグナル待ちでブロックされたスレッド」という扱いで、`pthread_cond_wait` や `read` のブロックと同じく、スケジューラ的にはスリープ状態に入っていると考えれば OK。
 *
 * ### スレッド全般
 *
 * - pthread_create
 *   - 新しいスレッドを生成する。このとき、親スレッドのシグナルマスクは子スレッドに継承される。
 * - pthread_kill
 *   - 特定のスレッドにシグナルを送る
 * - pthread_join
 *   - 子スレッドの終了を待機しリソースを回収する
 *
 * ### バリア系 pthread_barrier_xxx
 *
 * - pthread_barrier_t / pthread_barrier_init / pthread_barrier_wait
 *   - pthread_barrier_t は「指定した数のスレッドが同じ地点に到達するまで待つ」ための同期オブジェクト
 *   - pthread_barrier_init でカウント（待ち合わせるスレッド数）を設定して初期化し、それぞれのスレッドが
 *     pthread_barrier_wait を呼び出すと、全員が揃うまでブロックされる
 *   - 今回はメインスレッドとワーカースレッドの 2 本でカウントを 2 に設定し、準備完了のタイミングを合わせている
 *   - 使い終えたら pthread_barrier_destroy で破棄し、内部リソース（メモリ、システムオブジェクト）を解放する
 *
 * ## フロー概要
 *
 *  (1) メインスレッドで SIGINT をブロック (sigprocmask)
 *      - スレッド生成前なので sigprocmask を安全に使える
 *      - ここで設定したマスクは pthread_create した子スレッドにも継承される
 *  (2) ワーカースレッドを生成後、ワーカースレッドは SIGINT を待ち受ける準備をする
 *      - pthread_sigmask で自スレッドのマスクを確認/設定
 *      - sigwait で同期的にシグナルを受け取る
 *  (3) バリアで「両スレッドとも準備完了」まで待つ
 *  (4) ユーザーが外部から SIGINT を送る (例: kill -SIGINT <PID>)
 *  (5) ワーカースレッドはシグナルを受信し終了。メインは pthread_join で待つ
 *
 * ## シグナルの説明
 *
 * ref: https://qiita.com/Kernel_OGSun/items/e96cef5487e25517a576#1-%E3%82%B7%E3%82%B0%E3%83%8A%E3%83%AB%E3%81%A8%E3%81%AF
 *
 * シグナル signal は、プロセスやプロセスグループへ様々なイベントを通知するためにあるカーネルの機能 (ソフトウェア割り込み)。
 * イベントの通知は様々な場所 (自分 / 他プロセス、カーネル) から行うことが可能で、以下のようなことができる。
 *
 * - ハングしたプロセスにシグナルを送信して強制終了させる
 * - シグナルを送信してプロセスの処理を一時停止・再開させる
 * - ハードウェア例外 (0 除算、メモリアクセス違反など) 時にシグナルを送信してプロセスを終了させる
 * - シグナルを送信する特殊なキー (Ctrl + C など) を入力しプロセスを終了させる
 * - シグナル受信時にユーザ定義の関数 (シグナルハンドラ) を実行させる
 *
 * サンプルコードで利用するシグナルは以下の 2 つ。
 * - SIGINT
 *   - Ctrl+C や kill -SIGINT が発生させる割り込みシグナル。
 * - SIGUSR1
 *   - ユーザー定義用のシグナルで、用途をプログラム側が自由に決められる。
 *
 * ## Q & A
 *
 * - 処理されずにそのプロセスが kill されたらどのタイミングで未処理のシグナルは解放されるのだろうか。
 *   そもそもキュー自体がプロセスの持ち物なら kill と同時に解放される？キューは誰の持ち物？プロセスごと？スレッドグループごと？スレッドごと？
 *
 */

static void fatal(const char *msg, int err)
{
  fprintf(stderr, "%s: %s\n", msg, strerror(err));
  exit(EXIT_FAILURE);
}

struct thread_metadata {
  pthread_t parent_tid;
  const char *thread_name;
};

// -------------------------------------------------------
// SIGINT の挙動検証
// -------------------------------------------------------

static const int THREAD_NUMBER_TEST_SIGINT = 2; // SIGINT の待ち受けテストで待つスレッドの数
static pthread_barrier_t barrier_test_sigint;
static pthread_t worker_thread_test_sigint;
static const char *WORKER_NAME_TEST_SIGINT = "worker_test_SIGINT";

static void *worker_test_SIGINT(void *arg)
{
  // arg の受け取り
  struct thread_metadata *meta = arg;
  const char *worker_name = (meta && meta->thread_name) ? meta->thread_name : "worker";
  pthread_t parent_tid = meta ? meta->parent_tid : 0;

  // ワーカースレッド内で使用するシグナルマスクの初期化
  sigset_t waitset;
  sigemptyset(&waitset);

  // SIGINT をマスクにする
  sigaddset(&waitset, SIGINT);

  // pthread_sigmask:
  //   スレッド単位でシグナルマスクを操作する関数。sigprocmask と異なり、マルチスレッド環境で使用できる API。
  //   ここでは念のため SIGINT がブロックされていることを保証する。
  //   (sigprocmask で親が設定した状態を引き継いでいるはずだが、明示的に指定している)
  int err = pthread_sigmask(SIG_BLOCK, &waitset, NULL);
  if (err != 0)
    fatal("pthread_sigmask", err);

  printf("[worker:%s] ready to receive SIGINT (thread id %lu, parent %lu)\n",
         worker_name, (unsigned long)pthread_self(), (unsigned long)parent_tid);

  // pthread_barrier_wait:
  //   すべての参加スレッドがこの地点へ到達するまで待機するバリア同期。
  //   今回はメインスレッドとワーカースレッドの 2 つでカウントを設定している。
  err = pthread_barrier_wait(&barrier_test_sigint);
  if (err != 0 && err != PTHREAD_BARRIER_SERIAL_THREAD)
    fatal("pthread_barrier_wait (worker)", err);

  // sigwait:
  //   ブロックされているシグナルが届くまで待ち、そのシグナル番号を返す。
  //   pthread_sigmask で SIGINT をブロックしているので安全に同期的な待受ができる。
  int sig = 0;
  err = sigwait(&waitset, &sig);
  if (err != 0)
    fatal("sigwait", err);

  printf("[worker:%s] received signal %d, exiting\n", worker_name, sig);
  return NULL;
}

static void test_SIGINT(void)
{
  // シグナルマスクの初期化
  sigset_t blockset; // シグナル集合
  sigemptyset(&blockset);

  // SIGINT をマスクにする
  sigaddset(&blockset, SIGINT);
  sigset_t oldset;

  // メインスレッドのみの段階で sigprocmask を呼び、メインスレッドのシグナルをマスクする
  /* SIGBLOCK とは？

  sigprocmask 関数の第一引数 how は「どのようにマスクを変更するか」を伝える指定。指定できる値は次の 3 つ:

  - `SIG_BLOCK`
    - 第 2 引数の集合のビットを既存マスクに加える（OR する）。
    - 挙動としては `current_mask |= mask;`
    - 例: `sigprocmask(SIG_BLOCK, &blockset, &oldset)` は OR 演算「`現在のマスク |= blockset`」で現在のプロセスのシグナルマスクを更新、同時に元のマスクを `oldset` に保存。
  - `SIG_UNBLOCK`
    - 第 2 引数に含まれるビットを外す（差し引く）
    - 挙動としては `current_mask &= ~mask;`
    - e.g. current_mask = 0b0101, mask = 0b0001 のとき 0101 & ~(0001) = 0101 & 1110 = 0100
  - `SIG_SETMASK`
    - 第 2 引数で現在のマスクを丸ごと置き換える。
    - 挙動としては `current_mask = new_mask;`

  このように「足す／外す／置き換える」の操作を選ぶ必要があるため、第1引数が必須というわけです。

  - `/usr/include/aarch64-linux-gnu/bits/sigaction.h`

  ```
  Values for the HOW argument to `sigprocmask'.
  #define	SIG_BLOCK     0		 Block signals
  #define	SIG_UNBLOCK   1		 Unblock signals
  #define	SIG_SETMASK   2		 Set the set of blocked signals.
  ```
  */
  if (sigprocmask(SIG_BLOCK, &blockset, &oldset) == -1) {
    perror("sigprocmask");
    exit(EXIT_FAILURE);
  }

  // pthread_barrier_init:
  //   指定したカウント (今回は 2) のスレッドが待ち合わせに到達するまでブロックする同期オブジェクトを初期化。
  int err = pthread_barrier_init(&barrier_test_sigint, NULL, THREAD_NUMBER_TEST_SIGINT);
  if (err != 0)
    fatal("pthread_barrier_init", err);

  // pthread_create:
  //   新しいスレッドを生成し、worker を実行させる。
  //   親スレッドのシグナルマスクは子スレッドに引き継がれる点に注意。
  // - スレッドのメタデータの作成
  struct thread_metadata meta = {
    .parent_tid = pthread_self(),
    .thread_name = WORKER_NAME_TEST_SIGINT,
  };
  // - スレッドを生成
  err = pthread_create(&worker_thread_test_sigint, NULL, worker_test_SIGINT, &meta);
  if (err != 0)
    fatal("pthread_create", err);
  printf("[main] SIGINT worker thread spawned (tid %lu, pid %d)\n",
         (unsigned long)worker_thread_test_sigint, getpid());

  // pthread_barrier_wait:
  //   すべての参加スレッドがこの地点へ到達するまで待機するバリア同期。
  //   今回はメインスレッドとワーカースレッドの 2 つでカウントを設定している。
  //
  // 注意：
  // `pthread_barrier_wait` はシグナルとは無関係で、「参加している全スレッドが同じ位置まで到達したら全員を進める」だけの同期プリミティブ。
  //
  // 今回だと:
  // 1. メインがバリア地点まで進む → まだ人数ぶん揃っていないので待機。
  // 2. ワーカーが同じバリア地点に到達 → 参加数ぶん揃ったので pthread_barrier_wait を超えることができる。双方のバリアが同時に解除される。
  //
  // ミーティングの「全員集合したら開始」みたいな役割で、シグナルの配信順や受信状態とは切り離されている。
  printf("[main] waiting for worker to set up (barrier)...\n");
  err = pthread_barrier_wait(&barrier_test_sigint);
  if (err != 0 && err != PTHREAD_BARRIER_SERIAL_THREAD)
    fatal("pthread_barrier_wait (main)", err);

  // pthread_join:
  //   子スレッドの終了を待ち、リソースを回収する。
  //   join しないと「ゾンビスレッド」のようにリソースが残り続ける。
  printf("[main] please send SIGINT now by doing one of the following:\n- input Ctrl + C\n- execute command `kill -SIGINT %d` from another terminal\n", getpid());

  // ここで子スレッドの終了を待つ
  // sigwait 関数を使うことで、子スレッドはブロックされている（マスクされている）シグナルが届くまでスレッドをスリープ（ブロック）する
  err = pthread_join(worker_thread_test_sigint, NULL);
  if (err != 0)
    fatal("pthread_join", err);

  printf("[main] worker joined, cleaning up\n");

  // バリアのリソースを解放
  pthread_barrier_destroy(&barrier_test_sigint);

  // プロセスのシグナルマスクを元に戻す（ここであらかじめ保持しておいた oldset をシグナルマスクの復元に利用する）
  if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
    perror("sigprocmask restore");
    exit(EXIT_FAILURE);
  }
}

// -------------------------------------------------------
// SIGUSR1 の挙動検証
// -------------------------------------------------------

static const int THREAD_NUMBER_TEST_SIGUSR1 = 2; // SIGUSR1 の待ち受けテストで待つスレッドの数
static pthread_barrier_t barrier_test_sigusr1;
static pthread_t worker_thread_test_sigusr1;
static const char *WORKER_NAME_TEST_SIGUSR1 = "worker_test_SIGUSR1";

static void *worker_test_SIGUSR1(void *arg)
{
  struct thread_metadata *meta = arg;
  const char *worker_name = (meta && meta->thread_name) ? meta->thread_name : "worker";
  pthread_t parent_tid = meta ? meta->parent_tid : 0;

  sigset_t waitset;
  sigemptyset(&waitset);
  sigaddset(&waitset, SIGUSR1);

  int err = pthread_sigmask(SIG_BLOCK, &waitset, NULL);
  if (err != 0)
    fatal("pthread_sigmask", err);

  printf("[worker:%s] ready to receive SIGUSR1 (thread id %lu, parent %lu)\n",
         worker_name, (unsigned long)pthread_self(), (unsigned long)parent_tid);

  err = pthread_barrier_wait(&barrier_test_sigusr1);
  if (err != 0 && err != PTHREAD_BARRIER_SERIAL_THREAD)
    fatal("pthread_barrier_wait (worker SIGUSR1)", err);

  // SIGUSR1 を待ち受ける
  int sig = 0;
  err = sigwait(&waitset, &sig);
  if (err != 0)
    fatal("sigwait", err);

  printf("[worker:%s] received signal %d, exiting\n", worker_name, sig);
  return NULL;
}

static void test_SIGUSR1(void)
{
  // シグナルマスクの初期化
  sigset_t blockset;
  sigemptyset(&blockset);

  // SIGUSR1 をマスクする
  sigaddset(&blockset, SIGUSR1);

  // メインスレッドにシグナルマスクを設定
  sigset_t oldset; // 復元用のシグナルセットを定義
  if (sigprocmask(SIG_BLOCK, &blockset, &oldset) == -1) {
    perror("sigprocmask");
    exit(EXIT_FAILURE);
  }

  // バリアを初期化。メインスレッド + ワーカースレッドの計 2 つのスレッドを同期させるバリアを設定。
  int err = pthread_barrier_init(&barrier_test_sigusr1, NULL, THREAD_NUMBER_TEST_SIGUSR1);
  if (err != 0)
    fatal("pthread_barrier_init (SIGUSR1)", err);

  struct thread_metadata meta = {
      .parent_tid = pthread_self(),
      .thread_name = WORKER_NAME_TEST_SIGUSR1,
  };

  // ワーカースレッドの spawn
  err = pthread_create(&worker_thread_test_sigusr1, NULL, worker_test_SIGUSR1, &meta);
  if (err != 0)
    fatal("pthread_create (SIGUSR1)", err);

  printf("[main] SIGUSR1 worker thread spawned (tid %lu, pid %d)\n",
         (unsigned long)worker_thread_test_sigusr1, getpid());

  // バリアを使ってワーカースレッドのバリアの地点と同期
  err = pthread_barrier_wait(&barrier_test_sigusr1);
  if (err != 0 && err != PTHREAD_BARRIER_SERIAL_THREAD)
    fatal("pthread_barrier_wait (main SIGUSR1)", err);

  // バリアを抜けたら 3 秒後にワーカースレッドに SIGUSR1 を送信して kill
  printf("[main] sending SIGUSR1 to worker via pthread_kill in 3 seconds\n");
  for (int i = 3; i >= 1; i--) {
    printf("[main] countdown: %d\n", i);
    sleep(1);
  }
  err = pthread_kill(worker_thread_test_sigusr1, SIGUSR1);
  if (err != 0)
    fatal("pthread_kill", err);

  // ワーカースレッドの待ち受け
  err = pthread_join(worker_thread_test_sigusr1, NULL);
  if (err != 0)
    fatal("pthread_join (SIGUSR1)", err);
  printf("[main] worker joined, cleaning up\n");

  // バリアのリソースを解放
  pthread_barrier_destroy(&barrier_test_sigusr1);

  if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
    perror("sigprocmask restore");
    exit(EXIT_FAILURE);
  }
}

// -------------------------------------------------------
// entrypoint
// -------------------------------------------------------

int main(void)
{
  printf("----- test_SIGINT -----\n");
  // you must send signal SIGINT to this process by inputting Ctrl + C or command `kill -SIGINT <PID>`
  test_SIGINT();
  printf("\n");

  printf("----- test_SIGUSR1 -----\n");
  test_SIGUSR1();

  return EXIT_SUCCESS;
}
