#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INSTRUCTION_NUM 320//指令数量
#define PAGE_SIZE 10//页大小
#define PAGE_NUM (INSTRUCTION_NUM / PAGE_SIZE)//虚拟页数量
#define MIN_FRAME 4//最小物理页框数量
#define MAX_FRAME 32 //最大物理页框数量

static int rand_between(int left, int right)
{
    if (left > right) {
        return left;
    }
    return left + rand() % (right - left + 1);
}

//用来生成 [left, right] 范围内的随机整数。

static int contains(int frames[], int frame_count, int page)
{
    int i;

    for (i = 0; i < frame_count; i++) {
        if (frames[i] == page) {
            return i;
        }
    }
    return -1;
}

//用于判断 page 是否已经存在于 frames 数组中。是返回下标，不是就输入-1缺页

static int random_jump_address(int current)
{
    int choose_front = rand() % 2;

    if (choose_front) {
        return rand_between(0, current);
    }
    if (current + 2 <= INSTRUCTION_NUM - 1) {
        return rand_between(current + 2, INSTRUCTION_NUM - 1);
    }
    return rand_between(0, current);
}

//这里直接决定了之前和之后的概论是一半一半。
//用于生成一条随机跳转指令的地址。它随机决定是跳转到当前地址之前还是之后的某个地址。

static void generate_instructions(int instructions[], int sequential_percent)
{
    int i;
    int current = rand_between(0, INSTRUCTION_NUM - 1);

    instructions[0] = current; //第一条指令的地址是随机生成的

    for (i = 1; i < INSTRUCTION_NUM; i++) 
    {
        if ((rand() % 100) < sequential_percent) 
        {
            current = (current + 1) % INSTRUCTION_NUM;
            //顺序执行下一条指令。
        } 
        else 
        {
            current = random_jump_address(current);
            //生成一个随机跳转地址。
        }
        instructions[i] = current;
    }
}

//sequential_percent表示顺序执行的概论，它决定了生成的指令序列中有多少比例是顺序执行的
//generate_instructions用来生成 320 条指令访问序列

static void instructions_to_pages(const int instructions[], int pages[])
{
    int i;

    for (i = 0; i < INSTRUCTION_NUM; i++) {
        pages[i] = instructions[i] / PAGE_SIZE;
    }
}

//转换页号

static double fifo_hit_rate(const int pages[], int frame_count)
{
    int frames[MAX_FRAME];
    int loaded = 0;
    int next_replace = 0;
    int hits = 0;
    int i;

    for (i = 0; i < frame_count; i++) {
        frames[i] = -1;
    }

    for (i = 0; i < INSTRUCTION_NUM; i++) {
        if (contains(frames, loaded, pages[i]) != -1) {
            hits++;
            continue;
        }

        if (loaded < frame_count) {
            frames[loaded++] = pages[i];
        } else {
            frames[next_replace] = pages[i];
            next_replace = (next_replace + 1) % frame_count;
        }
    }

    return (double)hits / INSTRUCTION_NUM;
}

//FIFO 页面置换算法的命中率计算。当需要替换页面时，按照先进先出的原则进行替换

static double lru_hit_rate(const int pages[], int frame_count)
{
    int frames[MAX_FRAME];
    int last_used[MAX_FRAME];
    int loaded = 0;
    int hits = 0;
    int i;

    for (i = 0; i < frame_count; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    for (i = 0; i < INSTRUCTION_NUM; i++) {
        int pos = contains(frames, loaded, pages[i]);

        if (pos != -1) {
            hits++;
            last_used[pos] = i;

            continue;
        }

        if (loaded < frame_count) {
            frames[loaded] = pages[i];
            last_used[loaded] = i;
            loaded++;
        } else {
            int victim = 0;
            int j;

            for (j = 1; j < frame_count; j++) {
                if (last_used[j] < last_used[victim]) {
                    victim = j;
                }
            }
            frames[victim] = pages[i];
            last_used[victim] = i;
        }
    }

    return (double)hits / INSTRUCTION_NUM;
}

//LRU 页面置换算法的命中率计算。当需要替换页面时，选择最近最久未使用的页面进行替换

static double opt_hit_rate(const int pages[], int frame_count)
{
    int frames[MAX_FRAME];
    int loaded = 0;
    int hits = 0;
    int i;

    for (i = 0; i < frame_count; i++) {
        frames[i] = -1;
    }

    for (i = 0; i < INSTRUCTION_NUM; i++) {
        int pos = contains(frames, loaded, pages[i]);

        if (pos != -1) {
            hits++;
            continue;
        }

        if (loaded < frame_count) {
            frames[loaded++] = pages[i];
        } else {
            int victim = 0;
            int farthest = -1;
            int j;

            for (j = 0; j < frame_count; j++) {
                int k;
                int next_use = INSTRUCTION_NUM + 1;

                for (k = i + 1; k < INSTRUCTION_NUM; k++) {
                    if (pages[k] == frames[j]) {
                        next_use = k;
                        break;
                    }
                }

                if (next_use > farthest) {
                    farthest = next_use;
                    victim = j;
                }
            }
            frames[victim] = pages[i];
        }
    }

    return (double)hits / INSTRUCTION_NUM;
}

//OPT 页面置换算法的命中率计算。当需要替换页面时，选择未来最长时间内不会被访问的页面进行替换

static void print_experiment(const int pages[], int sequential_percent)
{
    int frame;

    printf("\nSequential execution ratio: %d%%\n", sequential_percent);
    printf("+--------+----------+----------+----------+\n");
    printf("| Frames |   OPT    |   FIFO   |   LRU    |\n");
    printf("+--------+----------+----------+----------+\n");

    for (frame = MIN_FRAME; frame <= MAX_FRAME; frame++) {
        double opt = opt_hit_rate(pages, frame);
        double fifo = fifo_hit_rate(pages, frame);
        double lru = lru_hit_rate(pages, frame);

        printf("| %6d | %7.2f%% | %7.2f%% | %7.2f%% |\n",
               frame, opt * 100.0, fifo * 100.0, lru * 100.0);
    }
    printf("+--------+----------+----------+----------+\n");
}

int main(int argc, char *argv[])
{
    int ratios[] = {25, 50, 75};
    int instructions[INSTRUCTION_NUM];//每条指令的地址

    int pages[INSTRUCTION_NUM];//每条指令对应的页号

    unsigned int seed;
    int i;

    if (argc > 1) {
        seed = (unsigned int)strtoul(argv[1], NULL, 10);
    } else {
        seed = (unsigned int)time(NULL);
    }
    srand(seed);

    printf("Page Replacement Simulation\n");
    printf("Instructions: %d, page size: %d, virtual pages: %d\n",
           INSTRUCTION_NUM, PAGE_SIZE, PAGE_NUM);
    printf("Random seed: %u\n", seed);

    for (i = 0; i < (int)(sizeof(ratios) / sizeof(ratios[0])); i++) 
    {
        generate_instructions(instructions, ratios[i]);
        instructions_to_pages(instructions, pages);
        print_experiment(pages, ratios[i]);
    }

    //每种概率都执行一遍
    
    return 0;
}
