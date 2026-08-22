#include <stdio.h>
#include <string.h>
#include "processflow.h"

Task tasks[MAX_TASKS];
int task_count = 0;   //initialize the task count to 0




void register_task(char *name, char *program, char *args[])
{
    if (task_count >= MAX_TASKS) {
        printf("Erro: limite de tarefas atingido.\n");
        return;
    }

    strcpy(tasks[task_count].name, name);
    strcpy(tasks[task_count].program, program);

    int i = 0;

    while (args[i] != NULL && i < MAX_ARGS) {
        strcpy(tasks[task_count].args[i], args[i]);
        i++;
    }

    tasks[task_count].arg_count = i;

    task_count++;
}