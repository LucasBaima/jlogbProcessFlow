#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "processflow.h"
#include <stdlib.h>

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

int execute_task(Task *task)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("Erro no fork");
        return 1;
    }

    if (pid == 0) {
        char *exec_args[MAX_ARGS + 1];

        for (int i = 0; i < task->arg_count; i++) {
            exec_args[i] = task->args[i];
        }

        exec_args[task->arg_count] = NULL;

        execv(task->program, exec_args);

        perror("Erro no exec");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    return 0;
}




void process_command(char *line)
{
    char *command = strtok(line, " \n");  // separa o comando em partes

    if (command == NULL) {
        return;
    }

    if (strcmp(command, "exit") == 0) {
        exit(0);
    }

    if (strcmp(command, "task") == 0) {
        char *name = strtok(NULL, " \n");
        char *program = strtok(NULL, " \n");

        if (name == NULL || program == NULL) {
            printf("Erro: task incompleta.\n");
            return;
        }

        char *args[MAX_ARGS];
        int i = 0;

        args[i++] = program;

        char *arg = strtok(NULL, " \n");

        while (arg != NULL && i < MAX_ARGS) {
            args[i++] = arg;
            arg = strtok(NULL, " \n");
        }

        args[i] = NULL;

        register_task(name, program, args);
        return;
    }

    if (strcmp(command, "run") == 0) {

        char *mode = strtok(NULL, " \n");

        if (mode == NULL) {
            printf("Erro: informe uma tarefa.\n");
            return;
        }

        /* run sequential ... */
        if (strcmp(mode, "sequential") == 0) {

            char *task_names[MAX_TASKS];
            int count = 0;

            char *name = strtok(NULL, " \n");

            while (name != NULL && count < MAX_TASKS) {
                task_names[count] = name;
                count++;

                name = strtok(NULL, " \n");
            }

            if (count == 0) {
                printf("Erro: nenhuma tarefa informada.\n");
                return;
            }

            run_sequential(task_names, count);
            return;
        }

        /* run parallel ... */
        if (strcmp(mode, "parallel") == 0) {

            char *task_names[MAX_TASKS];
            int count = 0;

            char *name = strtok(NULL, " \n");

            while (name != NULL && count < MAX_TASKS) {
                task_names[count] = name;
                count++;

                name = strtok(NULL, " \n");
            }

            if (count == 0) {
                printf("Erro: nenhuma tarefa informada.\n");
                return;
            }

            run_parallel(task_names, count);
            return;
        }

        /* run pipe tarefa1 tarefa2 ... */
        if (strcmp(mode, "pipe") == 0) {

            char *task_names[MAX_TASKS];
            int count = 0;

            char *name = strtok(NULL, " \n");

            while (name != NULL && count < MAX_TASKS) {
                task_names[count] = name;
                count++;

                name = strtok(NULL, " \n");
            }

            if (count < 2) {
                printf("Erro: informe pelo menos duas tarefas para o pipe.\n");
                return;
            }

            run_pipe(task_names, count);
            return;
        }

        
        /* run tarefa individual */
        Task *task = find_task(mode);

        if (task == NULL) {
            printf("Erro: tarefa nao encontrada.\n");
            return;
        }

        execute_task(task);
        return;
    }
}





Task *find_task(char *name)
{
    for (int i = 0; i < task_count; i++) {
        if (strcmp(tasks[i].name, name) == 0) {
            return &tasks[i];
        }
    }

    return NULL;
}



void run_sequential(char *task_names[], int count)
{
    for (int i = 0; i < count; i++) {

        Task *task = find_task(task_names[i]); // busca a tarefa pelo nome

        if (task == NULL) {
            printf("Erro: tarefa nao encontrada.\n");
            continue;
        }

        execute_task(task); // executa a tarefa
    }
}




void run_parallel(char *task_names[], int count)
{
    pid_t pids[MAX_TASKS];
    int created = 0;

    for (int i = 0; i < count; i++) {

        Task *task = find_task(task_names[i]);

        if (task == NULL) {
            printf("Erro: tarefa nao encontrada.\n");
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("Erro no fork");
            continue;
        }

        if (pid == 0) {
            char *exec_args[MAX_ARGS + 1];

            for (int j = 0; j < task->arg_count; j++) {
                exec_args[j] = task->args[j];
            }

            exec_args[task->arg_count] = NULL;

            execv(task->program, exec_args);

            perror("Erro no exec");
            exit(1);
        }

        pids[created] = pid;
        created++;
    }

    for (int i = 0; i < created; i++) {
        waitpid(pids[i], NULL, 0);
    }
}



void run_pipe(Task *first, Task *second)
{
    int fd[2];

    if (pipe(fd) < 0) {
        perror("Erro no pipe");
        return;
    }

    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("Erro no fork");
        return;
    }

    if (pid1 == 0) {
        char *args1[MAX_ARGS + 1];

        for (int i = 0; i < first->arg_count; i++) {
            args1[i] = first->args[i];
        }

        args1[first->arg_count] = NULL;

        dup2(fd[1], STDOUT_FILENO);

        close(fd[0]);
        close(fd[1]);

        execv(first->program, args1);

        perror("Erro no exec");
        exit(1);
    }

    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("Erro no fork");
        return;
    }

    if (pid2 == 0) {
        char *args2[MAX_ARGS + 1];

        for (int i = 0; i < second->arg_count; i++) {
            args2[i] = second->args[i];
        }

        args2[second->arg_count] = NULL;

        dup2(fd[0], STDIN_FILENO);

        close(fd[0]);
        close(fd[1]);

        execv(second->program, args2);

        perror("Erro no exec");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}