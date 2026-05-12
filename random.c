#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INSTR_NUM 320    // 指令总数
#define PAGE_NUM 32      // 总页数32页
#define PAGE_SIZE 10     // 每页10条指令

// 生成随机页地址流
void generate_random_stream(int page_stream[]) {
    srand((unsigned int)time(NULL));
    for (int i = 0; i < INSTR_NUM; i++) {
        int addr = rand() % INSTR_NUM;    // 0~319随机地址
        page_stream[i] = addr / PAGE_SIZE;
    }
}

// OPT最优置换
int opt(int pages[], int frame_num, int n) {
    int frame[PAGE_NUM] = {0};
    int count = 0, fault = 0;
    for (int i = 0; i < n; i++) {
        int find = 0;
        for (int j = 0; j < count; j++) {
            if (frame[j] == pages[i]) {
                find = 1;
                break;
            }
        }
        if (!find) {
            fault++;
            if (count < frame_num) frame[count++] = pages[i];
            else {
                int replace = 0, max = -1;
                for (int j = 0; j < frame_num; j++) {
                    int next = n;
                    for (int k = i + 1; k < n; k++) {
                        if (frame[j] == pages[k]) {
                            next = k;
                            break;
                        }
                    }
                    if (next > max) {
                        max = next;
                        replace = j;
                    }
                }
                frame[replace] = pages[i];
            }
        }
    }
    return fault;
}

// FIFO置换
int fifo(int pages[], int frame_num, int n) {
    int frame[PAGE_NUM] = {0};
    int count = 0, fault = 0, idx = 0;
    for (int i = 0; i < n; i++) {
        int find = 0;
        for (int j = 0; j < count; j++) {
            if (frame[j] == pages[i]) find = 1;
        }
        if (!find) {
            fault++;
            if (count < frame_num) frame[count++] = pages[i];
            else frame[idx++ % frame_num] = pages[i];
        }
    }
    return fault;
}

// LRU置换
int lru(int pages[], int frame_num, int n) {
    int frame[PAGE_NUM] = {0};
    int count = 0, fault = 0;
    int last[PAGE_NUM] = {0};
    for (int i = 0; i < n; i++) {
        int find = 0;
        for (int j = 0; j < count; j++) {
            if (frame[j] == pages[i]) {
                find = 1;
                last[j] = i;
                break;
            }
        }
        if (!find) {
            fault++;
            if (count < frame_num) {
                frame[count] = pages[i];
                last[count++] = i;
            } else {
                int min = 0;
                for (int j = 1; j < frame_num; j++) {
                    if (last[j] < last[min]) min = j;
                }
                frame[min] = pages[i];
                last[min] = i;
            }
        }
    }
    return fault;
}

int main() {
    int page_stream[INSTR_NUM];
    generate_random_stream(page_stream);

    printf("===== 随机序列 - 页面置换测试 =====\n");
    printf("内存块数\tOPT缺页数\tFIFO缺页数\tLRU缺页数\tOPT命中率\tFIFO命中率\tLRU命中率\n");

    // 测试4~32块内存
    for (int frame = 4; frame <= 32; frame++) {
        int f_opt = opt(page_stream, frame, INSTR_NUM);
        int f_fifo = fifo(page_stream, frame, INSTR_NUM);
        int f_lru = lru(page_stream, frame, INSTR_NUM);

        double h_opt = 1.0 - (double)f_opt / INSTR_NUM;
        double h_fifo = 1.0 - (double)f_fifo / INSTR_NUM;
        double h_lru = 1.0 - (double)f_lru / INSTR_NUM;

        printf("%d\t\t%d\t\t%d\t\t%d\t\t%.2f%%\t\t%.2f%%\t\t%.2f%%\n",
               frame, f_opt, f_fifo, f_lru,
               h_opt * 100, h_fifo * 100, h_lru * 100);
    }
    return 0;
}

