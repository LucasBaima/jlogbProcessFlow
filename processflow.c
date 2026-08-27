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


if (strcmp(command, "wait") == 0) {

    char *job_id_text = strtok(NULL, " \n");

    if (job_id_text == NULL) {
        printf("Erro: informe o job.\n");
        return;
    }

    int job_id = atoi(job_id_text);

    wait_job(job_id);

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



void run_pipe(char *task_names[], int count)
{
    Task *pipe_tasks[MAX_TASKS];
    pid_t pids[MAX_TASKS];
    int pipes[MAX_TASKS - 1][2];

    /* Primeiro procura todas as tarefas */
    for (int i = 0; i < count; i++) {
        pipe_tasks[i] = find_task(task_names[i]);

        if (pipe_tasks[i] == NULL) {
            printf("Erro: tarefa nao encontrada.\n");
            return;
        }
    }

    /* Cria os pipes necessários */
    for (int i = 0; i < count - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("Erro no pipe");
            return;
        }
    }

    /* Cria um processo para cada tarefa */
    for (int i = 0; i < count; i++) {

        pid_t pid = fork();

        if (pid < 0) {
            perror("Erro no fork");
            return;
        }

        if (pid == 0) {

            Task *task = pipe_tasks[i];

            char *exec_args[MAX_ARGS + 1];

            for (int j = 0; j < task->arg_count; j++) {
                exec_args[j] = task->args[j];
            }

            exec_args[task->arg_count] = NULL;


            /* Se não for a primeira tarefa,
               receber entrada do pipe anterior */
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // Se não for a última tarefa,
            //   enviar saída para o3 próximo pipe //
            if (i < count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }


            /* Fecha todos os descritores de pipe */
            for (int j = 0; j < count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }


            /* WORKDIR */
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

        pids[i] = pid;
    }


    // Pai não tá usando nenhum dos pipes */
    for (int i = 0; i < count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }


    /* Agora espera todos os processos */
    for (int i = 0; i < count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}



int start_task(Task *task)
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


        /* REDIRECIONAMENTO DE SAIDA */  //<-- aqui é onde o redirecionamento de saída é feito, verificando se o arquivo de saída foi especificado e se deve ser sobrescrito ou acrescentado.
        if (task->saida_arquivo[0] != '\0') {

            int fd_out;

            if (task->append_mode == 1) {
                fd_out = open(
                    task->saida_arquivo,
                    O_WRONLY | O_CREAT | O_APPEND,
                    0644
                );
            }
            else {
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


        /* WORKDIR */
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


    // PAI NAO ESPERA */

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


void wait_job(int job_id)
{
    for (int i = 0; i < job_count; i++) {

        if (jobs[i].id == job_id) {

            if (jobs[i].active == 0) {
                printf("Erro: job ja finalizado.\n");
                return;
            }

            waitpid(jobs[i].pid, NULL, 0);

            jobs[i].active = 0;

            return;
        }
    }

    printf("Erro: job nao encontrado.\n");
}