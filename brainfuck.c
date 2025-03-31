#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "brainfuck.h"

#define MEMORY_SIZE 30000
#define MAX_CODE_SIZE 1000000

void brainfuck_init(BrainFuck* bf) {
    if (!bf) return;
    memset(bf->memory, 0, MEMORY_SIZE);
    memset(bf->code, 0, MAX_CODE_LENGTH);
    memset(bf->brackets, -1, sizeof(int) * MAX_CODE_LENGTH);
    memset(bf->bracket_stack, 0, sizeof(int) * MAX_CODE_LENGTH);
    bf->pointer = 0;
    bf->code_pointer = 0;
    bf->bracket_count = 0;
}

static void find_brackets(BrainFuck* bf) {
    if (!bf) return;
    
    int* stack = (int*)malloc(sizeof(int) * MAX_CODE_LENGTH);
    if (!stack) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    
    int stack_top = 0;
    memset(bf->brackets, -1, sizeof(int) * MAX_CODE_LENGTH);
    
    for (int i = 0; bf->code[i] != '\0'; i++) {
        if (bf->code[i] == '[') {
            stack[stack_top++] = i;
        } else if (bf->code[i] == ']') {
            if (stack_top == 0) {
                fprintf(stderr, "Error: Unmatched ']' at position %d\n", i);
                free(stack);
                exit(1);
            }
            int start = stack[--stack_top];
            bf->brackets[start] = i;
            bf->brackets[i] = start;
        }
    }
    
    if (stack_top > 0) {
        fprintf(stderr, "Error: Unmatched '[' at position %d\n", stack[stack_top - 1]);
        free(stack);
        exit(1);
    }
    
    free(stack);
}

int brainfuck_load_file(BrainFuck* bf, const char* filename) {
    if (!bf || !filename) return 0;
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return 0;
    }

    size_t read = fread(bf->code, 1, MAX_CODE_LENGTH - 1, file);
    fclose(file);
    
    if (read >= MAX_CODE_LENGTH - 1) {
        fprintf(stderr, "Warning: File too large, truncated to %d bytes\n", MAX_CODE_LENGTH - 1);
    }
    
    bf->code[read] = '\0';
    return 1;
}

void brainfuck_run(BrainFuck* bf, const char* code) {
    if (!bf || !code) return;
    
    strncpy(bf->code, code, MAX_CODE_LENGTH - 1);
    bf->code[MAX_CODE_LENGTH - 1] = '\0';
    bf->code_pointer = 0;
    
    find_brackets(bf);
    
    while (bf->code[bf->code_pointer] != '\0') {
        char command = bf->code[bf->code_pointer];
        
        switch (command) {
            case '>':
                if (bf->pointer < MEMORY_SIZE - 1) {
                    bf->pointer++;
                } else {
                    bf->pointer = 0;
                }
                break;
                
            case '<':
                if (bf->pointer > 0) {
                    bf->pointer--;
                } else {
                    bf->pointer = MEMORY_SIZE - 1;
                }
                break;
                
            case '+':
                bf->memory[bf->pointer]++;
                break;
                
            case '-':
                bf->memory[bf->pointer]--;
                break;
                
            case '.':
                putchar(bf->memory[bf->pointer]);
                fflush(stdout);
                break;
                
            case ',':
                bf->memory[bf->pointer] = getchar();
                break;
                
            case '[':
                if (bf->memory[bf->pointer] == 0) {
                    bf->code_pointer = bf->brackets[bf->code_pointer];
                }
                break;
                
            case ']':
                if (bf->memory[bf->pointer] != 0) {
                    bf->code_pointer = bf->brackets[bf->code_pointer];
                }
                break;
        }
        
        bf->code_pointer++;
    }
}

int brainfuck_eval_line(BrainFuck* bf, const char* line) {
    if (!bf || !line) return 0;
    
    // 跳过空白行
    int has_valid_chars = 0;
    for (const char* p = line; *p; p++) {
        if (!isspace(*p)) {
            has_valid_chars = 1;
            break;
        }
    }
    if (!has_valid_chars) return 0;
    
    size_t len = strlen(line);
    for (size_t i = 0; i < len; i++) {
        char c = line[i];
        if (c == '[') {
            bf->bracket_stack[bf->bracket_count++] = strlen(bf->code);
        } else if (c == ']') {
            if (bf->bracket_count == 0) {
                fprintf(stderr, "Error: Unmatched ']'\n");
                return -1;
            }
            bf->bracket_count--;
        }
    }
    
    // 将新代码追加到现有代码中
    size_t current_len = strlen(bf->code);
    if (current_len + len >= MAX_CODE_LENGTH - 1) {
        fprintf(stderr, "Error: Code buffer overflow\n");
        return -1;
    }
    strcat(bf->code, line);
    
    // 如果所有括号都匹配，执行代码
    if (bf->bracket_count == 0) {
        bf->code_pointer = 0;
        find_brackets(bf);
        
        while (bf->code[bf->code_pointer] != '\0') {
            char command = bf->code[bf->code_pointer];
            
            switch (command) {
                case '>':
                    if (bf->pointer < MEMORY_SIZE - 1) {
                        bf->pointer++;
                    } else {
                        bf->pointer = 0;
                    }
                    break;
                    
                case '<':
                    if (bf->pointer > 0) {
                        bf->pointer--;
                    } else {
                        bf->pointer = MEMORY_SIZE - 1;
                    }
                    break;
                    
                case '+':
                    bf->memory[bf->pointer]++;
                    break;
                    
                case '-':
                    bf->memory[bf->pointer]--;
                    break;
                    
                case '.':
                    putchar(bf->memory[bf->pointer]);
                    fflush(stdout);
                    break;
                    
                case ',':
                    bf->memory[bf->pointer] = getchar();
                    break;
                    
                case '[':
                    if (bf->memory[bf->pointer] == 0) {
                        bf->code_pointer = bf->brackets[bf->code_pointer];
                    }
                    break;
                    
                case ']':
                    if (bf->memory[bf->pointer] != 0) {
                        bf->code_pointer = bf->brackets[bf->code_pointer];
                    }
                    break;
            }
            
            bf->code_pointer++;
        }
        // 执行完后清空代码缓冲区
        memset(bf->code, 0, MAX_CODE_LENGTH);
        return 1;
    }
    
    return 0;
}

void brainfuck_repl(BrainFuck* bf) {
    if (!bf) return;
    
    char line[MAX_LINE_LENGTH];
    printf("BrainFuck++ Interactive Shell\n");
    printf("Type 'exit' to quit\n\n");
    
    while (1) {
        if (bf->bracket_count > 0) {
            printf("... ");  // 继续输入提示符
        } else {
            printf(">>> ");  // 主提示符
        }
        fflush(stdout);
        
        if (!fgets(line, MAX_LINE_LENGTH, stdin)) {
            break;
        }
        
        // 移除换行符
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        // 检查退出命令
        if (strcmp(line, "exit") == 0 && bf->bracket_count == 0) {
            break;
        }
        
        // 执行代码
        int result = brainfuck_eval_line(bf, line);
        if (result == -1) {
            // 错误发生时重置状态
            memset(bf->code, 0, MAX_CODE_LENGTH);
            bf->bracket_count = 0;
        }
        
        if (result == 1) {
            printf("\n");  // 在执行结果后添加换行
        }
    }
}

void print_usage(const char* program) {
    printf("Usage: %s [file]\n", program);
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  file          Execute BrainFuck code from file\n");
    printf("  (no args)     Start interactive shell\n");
    printf("\nExamples:\n");
    printf("  %s                    # Start interactive shell\n", program);
    printf("  %s hello.bf           # Execute code from file\n", program);
    printf("  %s --help             # Show help message\n", program);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Cannot open file '%s'\n", argv[1]);
        return 1;
    }

    char *code = malloc(MAX_CODE_SIZE);
    if (!code) {
        printf("Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    size_t code_size = 0;
    int c;
    while ((c = fgetc(file)) != EOF && code_size < MAX_CODE_SIZE - 1) {
        // 跳过注释
        if (c == '#') {
            while ((c = fgetc(file)) != EOF && c != '\n');
            continue;
        }
        // 只保留有效的BrainFuck指令
        if (strchr("><+-.,[]", c)) {
            code[code_size++] = c;
        }
    }
    code[code_size] = '\0';
    fclose(file);

    // 初始化内存
    unsigned char *memory = calloc(MEMORY_SIZE, sizeof(unsigned char));
    if (!memory) {
        printf("Memory allocation failed\n");
        free(code);
        return 1;
    }

    // 执行代码
    execute(code, memory);

    // 清理
    free(code);
    free(memory);
    return 0;
}

void execute(const char* code, unsigned char* memory) {
    unsigned char* ptr = memory;
    const char* pc = code;
    int bracket_count;
    
    while (*pc) {
        switch (*pc) {
            case '>': ptr++; break;
            case '<': ptr--; break;
            case '+': (*ptr)++; break;
            case '-': (*ptr)--; break;
            case '.': putchar(*ptr); break;
            case ',': *ptr = getchar(); break;
            case '[':
                if (*ptr == 0) {
                    bracket_count = 1;
                    while (bracket_count > 0 && *pc) {
                        pc++;
                        if (*pc == '[') bracket_count++;
                        if (*pc == ']') bracket_count--;
                    }
                    if (bracket_count > 0) {
                        printf("Error: Unmatched '['\n");
                        return;
                    }
                }
                break;
            case ']':
                if (*ptr != 0) {
                    bracket_count = 1;
                    while (bracket_count > 0 && pc > code) {
                        pc--;
                        if (*pc == ']') bracket_count++;
                        if (*pc == '[') bracket_count--;
                    }
                    if (bracket_count > 0) {
                        printf("Error: Unmatched ']'\n");
                        return;
                    }
                    pc--;
                }
                break;
        }
        pc++;
    }
} 