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

    return 0;
}