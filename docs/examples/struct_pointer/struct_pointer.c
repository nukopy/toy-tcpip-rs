#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#ifndef DEVICE_NAME_LENGTH
#define DEVICE_NAME_LENGTH 32
#endif

#ifndef DEVICE_ADDRESS_LENGTH
#define DEVICE_ADDRESS_LENGTH 16
#endif

struct my_device {
    char name[DEVICE_NAME_LENGTH];
    unsigned int index;
    uint16_t addr[DEVICE_ADDRESS_LENGTH];
};

// -------------------------------------------------------
// utils
// -------------------------------------------------------

bool size_t_to_int(size_t s, int *out) {
    // size_t (unsigned long) が int に入るかどうかチェック
    // INT_MAX は当然 unsigned long に入るのでキャストして OK
    if (s > (size_t)INT_MAX) return false;

    // キャスト
    *out = (int)s;
    return true;
}

void print_device_info(const char *device_name, struct my_device *dev) {
    printf("%s: name=%s, index=%d\n", device_name, dev->name, dev->index);

    // get the number of items
    size_t n = sizeof dev->addr / sizeof dev->addr[0]; // 要素数

    // print items in array
    for (size_t i = 0; i < n; i++) {
        uint16_t item = dev->addr[i];
        printf("dev.addr[%zu] = %d\n", i, item);

        /* この丁寧なキャスト全くいらんかった。size_t のためのフォーマット文字列があった。
        int i_int;
        if (size_t_to_int(i, &i_int)) {
            printf("dev.addr[%d] = %d", i_int, item);
        } else {
            printf("Cannot cast i=%zu", i);
        }
        */
    }

    printf("\n");
}

// -------------------------------------------------------
// グローバルな構造体変数、静的な構造体変数の初期化時の挙動確認
// -------------------------------------------------------

// グローバルな構造体変数
struct my_device global_dev; // 自動的にゼロ値で初期化される

// 静的な構造体変数
static struct my_device static_dev;
// 各フィールドの初期値
// global_dev.name[0] = '\0' (空文字列)
// global_dev.index = 0
// global_dev.addr[0] = 0, addr[1] = 0, ... (全て 0)

void print_global_static_dev() {
    printf("----- グローバル変数、静的変数の初期化時の挙動確認 -----\n");

    // global_dev
    const char *name = "global_dev";
    print_device_info(name, &global_dev);

    // static_dev
    print_device_info("static_dev", &static_dev);

    printf("\n");
}

// -------------------------------------------------------
// グローバルな構造体ポインタ変数の挙動確認
// -------------------------------------------------------

// グローバルな構造体ポインタ変数
struct my_device *global_dev_ptr;

void print_global_dev_ptr() {
    printf("----- グローバルな構造体ポインタ変数の挙動確認 -----\n");

    // 文字列練習
    // char name[] = "global_dev_ptr"; // 配列 + 初期値（サイズは自動、"abc\0"）
    // char name[16] = "global_dev_ptr"; // 16バイト確保、残りは '\0' で埋まる
    // char name[16] = {0}; // 全ゼロ（空文字列）
    // const char *name = "global_dev_ptr"; // 書き換え不可のリテラルを指すポインタ

    // CAUTION: Segmentation fault
    char name[] = "global_dev_ptr"; // 配列 + 初期値（サイズは自動、"abc\0"）
    // print_device_info(name, global_dev_ptr);

    printf("\n");
}

// -------------------------------------------------------
// ローカルな構造体ポインタ変数の挙動確認
// -------------------------------------------------------

void print_local_dev_ptr() {
    printf("----- ローカルな構造体ポインタ変数の挙動確認 -----\n");

    // ローカルな構造体変数: ゼロ値による初期化はなく、ガベージ値が入る
    struct my_device local_dev;
    print_device_info("local_dev", &local_dev);

    // ローカルな構造体変数: 明示的にゼロ値で初期化（グローバルな）
    struct my_device local_dev_zero = {0};
    print_device_info("local_dev_zero", &local_dev_zero);

    // ローカルな構造体変数: 明示的に初期値を与える
    struct my_device local_dev_init = {"fibonacci", 17, {
        0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610,
    }};
    print_device_info("local_dev_init", &local_dev_init);

    // ローカルな構造体変数: 明示的に部分的な初期値を与える
    struct my_device local_dev_init_partial = {"fibonacci"};
    print_device_info("local_dev_init_partial", &local_dev_init_partial);

    // ローカルな構造体ポインタ
    // グローバルな構造体ポインタと異なり、構造体インスタンス分のメモリが確保され、ポインタ自体は有効になる
    // 🚨 ただし、ポインタの値、構造体インスタンスのフィールドの値には不正な値が入っているため、プログラムが壊れる可能性がある。
    /*
    未初期化ポインタが指す先（ランダムなアドレス）:
    ┌─────────────────────────────────┐
    │  name[32] (32バイト)             │ ← 不正アクセス！文字化けの原因
    ├─────────────────────────────────┤
    │  index (4バイト)                │ ← たまたま整数値として読める
    ├─────────────────────────────────┤
    │  addr[16] (32バイト)            │ ← たまたま数値として読める
    └─────────────────────────────────┘
    */
    // 現に print_device_info の出力が壊れている
    /*
    ev.addr[15] = 0          ← 前の出力の一部
    rtial: name=fibonacci    ← 別の出力の一部が混在
    の挙動確認 -----           ← 日本語文字列の一部
    , index=1667329647       ← 数値が単独で出力
    dev.addr[0] = 29194
    dev.addr[1] = 26996
    dev.addr[2] = 27745
    dev.addr[3] = 8250
    dev.addr[4] = 24942
    dev.addr[5] = 25965
    dev.addr[6] = 26173
    dev.addr[7] = 25193
    dev.addr[8] = 28271
    dev.addr[9] = 25441
    dev.addr[10] = 26979
    dev.addr[11] = 8236
    dev.addr[12] = 28265
    dev.addr[13] = 25956
    dev.addr[14] = 15736
    dev.addr[15] = 2608
     */
    /*

    # 何が起こっているか

    ## バッファオーバーフロー

    - dev->nameが指すランダムなメモリにNULL文字 (`\0`) がない
    - %s は NULL 文字まで読み続ける
    - 結果：メモリ境界を越えて大量の文字を読み取り

    ## 出力バッファの破壊

    - 長すぎる文字列が printf 内部のバッファを破壊
    - 後続の出力が文字化けや欠損を起こす
    - 文字列の途中で切れたり、他の出力と混在

    ## スタック破壊の可能性

    これは単なる「おかしな出力」ではなく危険なコードになっている：

    - メモリ破壊 - プログラムの安定性に影響
    - 予期しない動作 - 他の部分にも影響する可能性
    - セキュリティリスク - バッファオーバーフローは攻撃に利用される

    */
    // 構造体ポインタの NG な使い方
    struct my_device *local_dev_ptr;
    print_device_info("local_dev_ptr", local_dev_ptr);

    // ローカルな構造体ポインタ: NULL で初期化
    // CAUTION: 明示的に NULL で初期化すると Segmentation fault になる
    // struct my_device *local_dev_ptr_null = NULL;
    // print_device_info("local_dev_ptr_null", local_dev_ptr_null);

    // ローカルな構造体ポインタ: NULL で初期化し、メモリを確保する
    // Segmentation fault にならない！
    struct my_device *local_dev_ptr_null_malloc = NULL;
    // ヒープに確保
    local_dev_ptr_null_malloc = malloc(sizeof(struct my_device));
    print_device_info("local_dev_ptr_null_malloc", local_dev_ptr_null_malloc);
    free(local_dev_ptr_null_malloc);
    // free(local_dev_ptr_null_malloc); // 同じポインタに対して 2 回 free は実行できない。crash する。

    printf("\n");
}

// -------------------------------------------------------
// entrypoint
// -------------------------------------------------------

int main() {
    print_global_static_dev();
    print_global_dev_ptr();
    print_local_dev_ptr();

    return 0;
}
