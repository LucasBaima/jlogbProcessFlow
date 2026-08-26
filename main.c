#include <stdio.h>
#include "processflow.h"




int main(int argc, char *argv[])
{
    char line[256];

    if (argc > 2) {
        fprintf(stderr, "Erro: numero incorreto de argumentos.\n");
        return 1;
    }

    if (argc == 1) {
        while (1) {
            printf("processflow> ");

            if (fgets(line, sizeof(line), stdin) == NULL) { // ler a linha de comando
                break;
            }

            
            process_command(line); //chamada
        }
    }

    if (argc == 2) {
         FILE *workflow = fopen(argv[1], "r");

    if (workflow == NULL) {
        perror("Erro ao abrir workflow");
        return 1;
    }

    while (fgets(line, sizeof(line), workflow) != NULL) { // ler cada linha do arquivo de workflow

        printf("%s", line);

        process_command(line);
    }

    fclose(workflow);
    }

    return 0;
}