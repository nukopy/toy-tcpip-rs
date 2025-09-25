#include <stdio.h>

int main() {
    // 溢れないパターン
    // note: char 型は 1 byte = 8 bit (標準では「少なくとも 8 ビット」)
    // note: {'d','u','m','m','y','\0',0,0,0,0,0,0,0,0,0,0}
    char name[16];
    snprintf(name, sizeof(name), "dummy");
    printf("name: %s\n", name);

    // 溢れるパターン
    // note: {'T','h','i','s',' ','n','a','m','e',' ','i','s',' ','t','o','\0'}
    // 20 文字の文字列は配列サイズを超えるので終端前で切れる。
    // snprintf は書き込み総数を返すので truncation の検出に使える。
    int written = snprintf(name, sizeof(name), "This name is too long");
    if (written >= (int)sizeof(name)) {
        printf("name was truncated: written=%d, name=\"%s\"\n", written, name);
    }

    return 0;
}
