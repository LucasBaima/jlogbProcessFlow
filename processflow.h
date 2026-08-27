#ifndef PROCESSFLOW_H
#define PROCESSFLOW_H

#define MAX_TASKS 100
#define MAX_NAME 50
#define MAX_PROGRAM 200
#define MAX_ARGS 20
#define MAX_ARG_LEN 100
#define MAX_FILE 200 // 

#define MAX_WORKDIR 300 // tamanho máximo do caminho do diretório de trabalho
#define MAX_JOBS 100 // tamanho máximo do caminho do diretório de trabalho


typedef struct {
    char name[MAX_NAME];
    char program[MAX_PROGRAM];
    char args[MAX_ARGS][MAX_ARG_LEN];  // guarda os argumentos dentro dela mesma
    int arg_count;

    char entrada_arquivo[MAX_FILE]; // arquivo de entrada
    char saida_arquivo[MAX_FILE]; // arquivo de saída
    int append_mode; // 0 para sobrescrever, 1 para acrescentar
} Task;

typedef struct {
    int id;
    pid_t pid;
    int active;
} Job;




extern Task tasks[MAX_TASKS];   //<-- extern declara uma variável global sem alocar ela ali; a definição real fica em um único .c.
extern int task_count;
extern char current_workdir[MAX_WORKDIR]; // variável global para armazenar o diretório de trabalho atual

extern Job jobs[MAX_JOBS]; // variável global para armazenar as tarefas em execução
extern int job_count;

extern char current_workdir[MAX_WORKDIR];

void register_task(char *name, char *program, char *args[]);

int execute_task(Task *task);

int start_task(Task *task);

void list_jobs(void);

void wait_job(int job_id);

void process_command(char *line);

Task *find_task(char *name);

void run_sequential(char *task_names[], int count);

void run_parallel(char *task_names[], int count);

void run_pipe(char *task_names[], int count) // função para executar tarefas em paralelo com pipe




#endif


//Recapitulando -> Background executar sem bloquear o ProcessFlow; O processo continua rodando enquanto o
//usuário tem a possibilidade de digitar novos comandos!

//PID → identificador dado pelo Sistema Operacional ao processo.
//Job ID → número que o nosso próprio ProcessFlow cria para organizar os processos em background.