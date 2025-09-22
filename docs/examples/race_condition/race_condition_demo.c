#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

struct net_device {
    unsigned int index;
    char name[16];
};

// 問題のあるコード（元の net_device_register風）
int unsafe_register(struct net_device *dev) {
    static unsigned int index = 0;

    // 危険な箇所：読み取り → 使用 → 更新の間に他のスレッドが割り込む可能性
    unsigned int current_index = index;  // 読み取り
    usleep(1000000);  // 意図的な遅延（競合状態を発生させやすくする）
    dev->index = current_index;          // 使用
    index = current_index + 1;           // 更新

    snprintf(dev->name, sizeof(dev->name), "net%d", dev->index);
    printf("Thread registered device: %s\n", dev->name);

    return 0;
}

pthread_mutex_t mymutex;

int safe_register(struct net_device *dev) {
    pthread_mutex_lock(&mymutex);
    static unsigned int index = 0;

    // 危険な箇所：読み取り → 使用 → 更新の間に他のスレッドが割り込む可能性
    unsigned int current_index = index;  // 読み取り
    usleep(1000000);  // 意図的な遅延（競合状態を発生させやすくする）
    dev->index = current_index;          // 使用
    index = current_index + 1;           // 更新

    snprintf(dev->name, sizeof(dev->name), "net%d", dev->index);
    printf("Thread registered device: %s\n", dev->name);
    pthread_mutex_unlock(&mymutex);

    return 0;
}

void* thread_function(void* arg) {
    int thread_id = *(int*)arg;
    struct net_device dev;

    printf("Thread %d starting registration\n", thread_id);
    unsafe_register(&dev);
    printf("Thread %d finished: index=%d, name=%s\n", thread_id, dev.index, dev.name);

    return NULL;
}

void* thread_function_safe(void* arg) {
    int thread_id = *(int*)arg;
    struct net_device dev;

    printf("Thread %d starting registration\n", thread_id);
    safe_register(&dev);
    printf("Thread %d finished: index=%d, name=%s\n", thread_id, dev.index, dev.name);

    return NULL;
}

int unsafe_condtion() {
    printf("=== マルチスレッドでの競合状態デモ ===\n");

    pthread_t threads[5];
    int thread_ids[5];

    // 5つのスレッドを作成
    printf("スレッドを作成中...\n");
    for (int i = 0; i < 5; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
    }
    printf("スレッドの作成が完了しました\n");

    // すべてのスレッドの終了を待つ
    printf("スレッドの終了を待ちます...\n");
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("DONE!\n");

    return 0;
}

int safe_condition() {
    printf("=== マルチスレッドでの競合状態デモ（スレッドセーフ版） ===\n");

    pthread_t threads[5];
    int thread_ids[5];

    // 5つのスレッドを作成
    printf("スレッドを作成中...\n");
    for (int i = 0; i < 5; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function_safe, &thread_ids[i]);
    }
    printf("スレッドの作成が完了しました\n");

    // すべてのスレッドの終了を待つ
    printf("スレッドの終了を待ちます...\n");
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("DONE!\n");

    return 0;
}

int main() {
    unsafe_condtion();
    safe_condition();

    return 0;
}
