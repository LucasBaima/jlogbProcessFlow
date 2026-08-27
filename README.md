# jlogb# ProcessFlow

Implementação da atividade **Orquestrador de Processos** da disciplina de Infraestrutura de Software.

O ProcessFlow foi desenvolvido em C e permite cadastrar tarefas que representam programas existentes no sistema e executá-las através de processos filhos.

## Funcionalidades implementadas

* Cadastro e execução de tarefas;
* Execução simples;
* Execução sequencial e paralela;
* Pipes entre processos;
* Redirecionamento de entrada e saída (`input`, `output` e `append`);
* Alteração do diretório de trabalho com `workdir`;
* Execução em background com `start`;
* Consulta de jobs com `jobs`;
* Espera de um job específico com `wait`;
* Modo interativo;
* Execução através de arquivos workflow `.pf`;
* Tratamento básico de comandos e erros.

O programa foi compilado e testado em **Debian 13**.

## Compilação

```bash
make clean
make
```

O executável gerado será:

```bash
./processflow
```

## Modo interativo

```bash
./processflow
```

Exemplo:

```text
task listar /bin/ls -l
run listar
exit
```

## Modo workflow

```bash
./processflow arquivo.pf
```

Nesse modo, os comandos são lidos diretamente do arquivo `.pf`.

## Evidências

O arquivo `evidencias.log` contém registros de compilação e testes realizados no ambiente Linux.

## Repositório

https://github.com/LucasBaima/jlogbProcessFlow
