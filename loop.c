#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define INSTRUCTION_NUM 320
#define PAGE_SIZE 10
#define FRAME_START 4
#define FRAME_END   32

// ================= 模式选择 =================
// 1 = 25% 顺序
// 2 = 50% 顺序（默认）
// 3 = 75% 顺序
#define SEQ_MODE 2
// ============================================

// 严格按照题目7步规则生成，且支持25%/50%/75%
void generate_sequence(int inst[], int n) {
    int i = 0;
    int s = rand() % n;

    while (i < n) {
        // 执行 s
        if (i < n) inst[i++] = s;

        // 顺序 s+1
        if (i < n) inst[i++] = s + 1;

        // 75% 模式：再多执行一条顺序
        if (SEQ_MODE == 3 && i < n) inst[i++] = s + 2;

        // 前地址 [0, s]
        int m = rand() % (s + 1);
        if (i < n) inst[i++] = m;

        // 25% 模式：去掉这条顺序
        if (SEQ_MODE != 1 && i < n) inst[i++] = m + 1;

        // 新 s 从后地址取
        int low = m + 2;
        if (low >= n) low = n - 1;
        s = low + rand() % (n - low);
    }
}

// 指令转页号
void inst_to_page(int inst[], int page[], int n) {
    for (int i = 0; i < n; i++)
        page[i] = inst[i] / PAGE_SIZE;
}

// FIFO
int fifo(int page[], int n, int frame_num) {
    int *frames = malloc(frame_num * sizeof(int));
    memset(frames, -1, frame_num * sizeof(int));
    int idx = 0, fault = 0;

    for (int i = 0; i < n; i++) {
        int p = page[i];
        int hit = 0;
        for (int j = 0; j < frame_num; j++)
            if (frames[j] == p) { hit = 1; break; }
        if (hit) continue;

        fault++;
        frames[idx] = p;
        idx = (idx + 1) % frame_num;
    }
    free(frames);
    return fault;
}

// LRU
int lru(int page[], int n, int frame_num) {
    int *frames = malloc(frame_num * sizeof(int));
    int *used = malloc(frame_num * sizeof(int));
    memset(frames, -1, frame_num * sizeof(int));
    memset(used, 0, frame_num * sizeof(int));
    int fault = 0, time = 0;

    for (int i = 0; i < n; i++) {
        int p = page[i];
        int hit = 0;
        for (int j = 0; j < frame_num; j++)
            if (frames[j] == p) { hit = 1; used[j] = ++time; break; }
        if (hit) continue;

        fault++;
        int lru_idx = 0;
        for (int j = 1; j < frame_num; j++)
            if (used[j] < used[lru_idx]) lru_idx = j;
        frames[lru_idx] = p;
        used[lru_idx] = ++time;
    }
    free(frames);
    free(used);
    return fault;
}

// OPT
int opt(int page[], int n, int frame_num) {
    int *frames = malloc(frame_num * sizeof(int));
    memset(frames, -1, frame_num * sizeof(int));
    int fault = 0;

    for (int i = 0; i < n; i++) {
        int p = page[i];
        int hit = 0;
        for (int j = 0; j < frame_num; j++)
            if (frames[j] == p) { hit = 1; break; }
        if (hit) continue;

        fault++;
        int empty = -1;
        for (int j = 0; j < frame_num; j++)
            if (frames[j] == -1) { empty = j; break; }
        if (empty != -1) { frames[empty] = p; continue; }

        int farthest = i, replace = 0;
        for (int j = 0; j < frame_num; j++) {
            int k;
            for (k = i+1; k < n && page[k] != frames[j]; k++);
            if (k > farthest) { farthest = k; replace = j; }
        }
        frames[replace] = p;
    }
    free(frames);
    return fault;
}

void run(int page[], int n) {
    char *mode_str;
    if (SEQ_MODE == 1) mode_str = "25% 顺序";
    else if (SEQ_MODE == 3) mode_str = "75% 顺序";
    else mode_str = "50% 顺序";

    printf("===== 当前模式：%s =====\n", mode_str);
    printf("页框 | FIFO缺页 | 命中率 | LRU缺页 | 命中率 | OPT缺页 | 命中率\n");
    printf("-------------------------------------------------------------\n");

    for (int f = FRAME_START; f <= FRAME_END; f++) {
        int ff = fifo(page, n, f);
        int lf = lru(page, n, f);
        int of = opt(page, n, f);

        double fh = 100.0 - (double)ff / n * 100;
        double lh = 100.0 - (double)lf / n * 100;
        double oh = 100.0 - (double)of / n * 100;

        printf("%4d | %8d | %6.2f%% | %7d | %6.2f%% | %7d | %6.2f%%\n",
               f, ff, fh, lf, lh, of, oh);
    }
}

int main() {
    srand(time(NULL));
    int inst[INSTRUCTION_NUM];
    int page[INSTRUCTION_NUM];

    generate_sequence(inst, INSTRUCTION_NUM);
    inst_to_page(inst, page, INSTRUCTION_NUM);
    run(page, INSTRUCTION_NUM);

    return 0;
}

