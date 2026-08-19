#include <stdio.h>
#include "processflow.h"




int main(int argc, char *argv[]) //Qtd de argumentos e vetor de argumentos -> comandos terminal
{
    if (argc > 2) {
        printf("Erro: numero incorreto de argumentos.\n");
        return 1;
    }

    if (argc == 1) {   //INTERATIVO
        printf("Modo interativo\n");
        printf("processflow> ");
    }

    if (argc == 2) { //WORKFLOW
        printf("Modo workflow: %s\n", argv[1]);
    }

    return 0;
}