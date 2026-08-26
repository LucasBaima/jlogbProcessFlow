#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "processflow.h"
#include <stdlib.h>
#include <fcntl.h> // for open() and O_* constants

Task tasks[MAX_TASKS];
int task_count = 0;   //initialize the task count to 0


char current_workdir[MAX_WORKDIR] = "";

Job jobs[MAX_JOBS];
int job_count = 0;



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

    tasks[task_count].entrada_arquivo[0] = '\0';
    tasks[task_count].saida_arquivo[0] = '\0';
    tasks[task_count].append_mode = 0;

    task_count++;
}



int execute_task(Task *task)  //pai usa waitpid
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


        /* REDIRECIONAMENTO DE ENTRADA */
        if (task->entrada_arquivo[0] != '\0') {

            int fd_in = open(task->entrada_arquivo, O_RDONLY);

            if (fd_in < 0) {
                perror("Erro ao abrir arquivo de entrada");
                exit(1);
            }

            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }


        /* REDIRECIONAMENTO DE SAIDA */
        if (task->saida_arquivo[0] != '\0') {

            int fd_out;

            if (task->append_mode == 1) {

                fd_out = open(
                    task->saida_arquivo,
                    O_WRONLY | O_CREAT | O_APPEND,
                    0644
                );

            } else {

                fd_out = open(
                    task->saida_arquivo,
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644
                );
            }

            if (fd_out < 0) {
                perror("Erro ao abrir arquivo de saida");
                exit(1);
            }

            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }


        /* DIRETORIO DE TRABALHO */
        if (current_workdir[0] != '\0') {

            if (chdir(current_workdir) != 0) {
                perror("Erro ao mudar diretorio");
                exit(1);
            }
        }


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

    //implementação dos comandos input, output e append
// -----------------------------------------------------------------------------------------------------------

if (strcmp(command, "input") == 0) {
    char *task_name = strtok(NULL, " \n");
    char *file_name = strtok(NULL, " \n");

    if (task_name == NULL || file_name == NULL) {
        printf("Erro: input incompleto.\n");
        return;
    }

    Task *task = find_task(task_name);

    if (task == NULL) {
        printf("Erro: tarefa nao encontrada.\n");
        return;
    }

    strcpy(task->entrada_arquivo, file_name);
    return;
}


if (strcmp(command, "output") == 0) {
    char *task_name = strtok(NULL, " \n");
    char *file_name = strtok(NULL, " \n");

    if (task_name == NULL || file_name == NULL) {
        printf("Erro: output incompleto.\n");
        return;
    }

    Task *task = find_task(task_name);

    if (task == NULL) {
        printf("Erro: tarefa nao encontrada.\n");
        return;
    }

    strcpy(task->saida_arquivo, file_name);
    task->append_mode = 0;

    return;
}


if (strcmp(command, "append") == 0) {
    char *task_name = strtok(NULL, " \n");
    char *file_name = strtok(NULL, " \n");

    if (task_name == NULL || file_name == NULL) {
        printf("Erro: append incompleto.\n");
        return;
    }

    Task *task = find_task(task_name);

    if (task == NULL) {
        printf("Erro: tarefa nao encontrada.\n");
        return;
    }

    strcpy(task->saida_arquivo, file_name);
    task->append_mode = 1;

    return;
}

if (strcmp(command, "workdir") == 0) {
    char *dir = strtok(NULL, " \n");

    if (dir == NULL) {
        printf("Erro: informe um diretorio.\n");
        return;
    }

    if (access(dir, F_OK) != 0) {
        printf("Erro: diretorio nao encontrado.\n");
        return;
    }

    strcpy(current_workdir, dir);
    return;
}

if (strcmp(command, "start") == 0) {
    char *task_name = strtok(NULL, " \n");

    if (task_name == NULL) {
        printf("Erro: informe uma tarefa.\n");
        return;
    }

    Task *task = find_task(task_name);

    if (task == NULL) {
        printf("Erro: tarefa nao encontrada.\n");
        return;
    }

    start_task(task);
    return;
}


if (strcmp(command, "jobs") == 0) {
    list_jobs();
    return;
}
// -----------------------------------------------------------------------------------------

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



int start_task(Task *task)  // O pai salva o PID do filho -- processo está em jobs[] para acompanhamento
{
    pid_t pid = fork(); // fork cria um novo processo, que é uma cópia do processo atual. O novo processo é chamado de processo filho,
    // e o processo original é chamado de processo pai. A função fork retorna o PID (Process ID) do processo
    // filho para o processo pai, e retorna 0 para o processo filho.    <---- LEMBRAR

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

        if (current_workdir[0] != '\0') {
            if (chdir(current_workdir) != 0) {
                perror("Erro ao mudar diretorio");
                exit(1);
            }
        }

        execv(task->program, exec_args);

        perror("Erro no exec");
        exit(1);
    }

    // PAI NÃO ESPERA */     <-- LEMBRAR

    if (job_count >= MAX_JOBS) {
        printf("Erro: limite de jobs atingido.\n");
        return 1;
    }

    jobs[job_count].id = job_count + 1;
    jobs[job_count].pid = pid;
    jobs[job_count].active = 1;

    printf("[%d] %d\n",
           jobs[job_count].id,
           jobs[job_count].pid);

    job_count++;

    return 0;
}



void list_jobs(void)
{
    for (int i = 0; i < job_count; i++) {

        if (jobs[i].active == 1) {

            int status;

            pid_t result = waitpid( 
                jobs[i].pid,
                &status,
                WNOHANG // Verifica se o processo terminou sem bloquear a execução do programa. 
            );

            if (result == 0) {
                /* processo ainda esta rodando */
                printf("[%d] %d\n",
                       jobs[i].id,
                       jobs[i].pid);
            }

            else if (result == jobs[i].pid) {
                /* processo ja terminou */
                jobs[i].active = 0;
            }
        }
    }
}