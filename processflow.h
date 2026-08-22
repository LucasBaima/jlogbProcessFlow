#ifndef PROCESSFLOW_H
#define PROCESSFLOW_H

#define MAX_TASKS 100
#define MAX_NAME 50
#define MAX_PROGRAM 200
#define MAX_ARGS 20
#define MAX_ARG_LEN 100


typedef struct {
    char name[MAX_NAME];
    char program[MAX_PROGRAM];
    char args[MAX_ARGS][MAX_ARG_LEN];  // guarda os argumentos dentro dela mesma
    int arg_count;
} Task;

extern Task tasks[MAX_TASKS];   //<-- extern declara uma variável global sem alocar ela ali; a definição real fica em um único .c.
extern int task_count;

void register_task(char *name, char *program, char *args[]);

#endif