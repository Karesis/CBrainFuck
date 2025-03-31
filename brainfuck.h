#ifndef BRAINFUCK_H
#define BRAINFUCK_H

#define MEMORY_SIZE 30000
#define MAX_CODE_LENGTH 1000000
#define MAX_LINE_LENGTH 1024

typedef struct {
    unsigned char memory[MEMORY_SIZE];
    int pointer;
    char code[MAX_CODE_LENGTH];
    int code_pointer;
    int brackets[MAX_CODE_LENGTH];
    int bracket_stack[MAX_CODE_LENGTH];  // 用于交互模式下的括号栈
    int bracket_count;                   // 当前未匹配的括号数
} BrainFuck;

// 初始化解释器
void brainfuck_init(BrainFuck* bf);

// 运行 BrainFuck 代码
void brainfuck_run(BrainFuck* bf, const char* code);

// 从文件读取 BrainFuck 代码
int brainfuck_load_file(BrainFuck* bf, const char* filename);

// 交互式环境相关函数
void brainfuck_repl(BrainFuck* bf);
int brainfuck_eval_line(BrainFuck* bf, const char* line);

void execute(const char* code, unsigned char* memory);

#endif // BRAINFUCK_H 