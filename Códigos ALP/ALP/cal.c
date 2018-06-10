/***********************************************************************
 * Code listing from "Advanced Linux Programming," by CodeSourcery LLC  *
 * Copyright (C) 2001 by New Riders Publishing                          *
 * See COPYRIGHT for license information.                               *
 ***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <wctype.h>

#include "server.h"

/* HTML source for the start of the page we generate.  */
extern char* params;

static char* page_start =
        "<html>\n"
        "<head>\n"
        "    <meta charset=\"utf-8\">\n"
        "</head>\n"
        " <body>\n"
        "  <pre>\n";

/* HTML source for the end of the page we generate.  */

static char* page_end =
        "  </pre>\n"
        " </body>\n"
        "</html>\n";

void module_generate(int fd) {
    pid_t child_pid;
    int rval;

    /* Write the start of the page.  */
    write(fd, page_start, strlen(page_start));
    /* Fork a child process.  */
    child_pid = fork();
    if (child_pid == 0) {
        /* This is the child process.  */
        /* Set up an argumnet list for the invocation of df.  */
        // ARRUMAR OS ARGUMENTOS AQUI PASSADOS COMO GET PELA URL    
        //char* argv[] = { "/bin/df", "-T", "-h", NULL };

        char* ano = NULL;
        char* mes = NULL;
        int a, m, flag = 0;
        char *aux = malloc(sizeof (params));
        strcpy(aux, params);
        if (strcmp(aux, "")) {
            mes = malloc(sizeof (aux));
            ano = malloc(sizeof (aux));
            strcpy(ano, aux);
            ano = strstr(ano, "ano");
            if (ano != NULL) {
                ano = strchr(ano, '=') + 1;
                strtok(ano, "&");
                a = atoi(ano);
            }
            strcpy(mes, aux);
            mes = strstr(mes, "mes");
            if (mes != NULL) {
                mes = strchr(mes, '=') + 1;
                m = atoi(mes);
                strtok(mes, "&");
            }
            if (strstr(aux, "&") != NULL) {
                flag = 1;
            }

        }
        //char* argv[] = {"/bin/echo", ano, NULL};


        //        char* argv[] = {"/usr/bin/cal", "-h", mes, ano, NULL};

        /* Duplicate stdout and stderr to send data to the client socket.  */
        rval = dup2(fd, STDOUT_FILENO);
        if (rval == -1)
            system_error("dup2");
        rval = dup2(fd, STDERR_FILENO);
        if (rval == -1)
            system_error("dup2");
        /* Run df to show the free space on mounted file systems.  */
        //   execv(argv[0], argv);

        if (mes == NULL) {
            if (flag == 1) {
                write(fd, "<div style='background: red; margin: auto; border-radius: 10%; text-align: center; width: 80%;'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n", strlen("<div style='background: red; margin: auto; border-radius: 10%; text-align: center; width: 80%;'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n"));
            }
            if (ano == NULL) {
                char* argv[] = {"/usr/bin/cal", "-h", NULL};
                execv(argv[0], argv);
            } else {
                if (a < 0 || a > 9999 || a == NULL) {
                    write(fd, "Formato incorreto\n", strlen("Formato incorreto\n"));
                    write(fd, "Ano deve ser um número entre 0 e 9999\n", strlen("Ano deve ser um número entre 0 e 9999\n"));
                    char* argv[] = {"/usr/bin/cal", "-h", NULL};
                    execv(argv[0], argv);
                } else {
                    char* argv[] = {"/usr/bin/cal", "-h", ano, NULL};
                    execv(argv[0], argv);
                }
            }
        } else {
            if (m < 1 || m > 12 || m == NULL) {
                write(fd, "<h1>Formato incorreto</h1>\n", strlen("Formato incorreto\n"));
                write(fd, "Mes deve ser um número entre 1 e 12\n", strlen("Mes deve ser um número entre 1 e 12\n"));
                char* argv[] = {"/usr/bin/cal", "-h", NULL};
                execv(argv[0], argv);
            } else {
                if (ano == NULL) {
                    if (flag == 1) {
                        write(fd, "<div style='background: red; margin: auto; text-align: center; width: 80%; border-radius: 10%'><h1>Os parâmetros corretos são: ano=ano&mes=mes</h1></div>\n\n", strlen("<div style='background:red; margin: auto; text-align: center; width: 80%; border-radius: 10%'><h1>Os parametros corretos são ano=ano&mes=mes</h1></div>\n"));
                    }
                    char* argv[] = {"/usr/bin/cal", "-h", "-m", mes, NULL};
                    execv(argv[0], argv);
                } else {
                    if (a < 0 || a > 9999 || a == NULL) {
                        write(fd, "<strong>Formato incorreto</strong>\n", strlen("Formato incorreto\n"));
                        write(fd, "Ano deve ser um número entre 0 e 9999\n", strlen("Ano deve ser um número entre 0 e 9999\n"));
                        char* argv[] = {"/usr/bin/cal", "-h", NULL};
                        execv(argv[0], argv);
                    } else {
                        char* argv[] = {"/usr/bin/cal", "-h", mes, ano, NULL};
                        execv(argv[0], argv);
                    }
                }
            }
        }

        /* A call to execv does not return unless an error occurred.  */
        system_error("execv");
    } else if (child_pid > 0) {
        /* This is the parent process.  Wait for the child process to
           finish.  */
        rval = waitpid(child_pid, NULL, 0);
        if (rval == -1)
            system_error("waitpid");
    } else
        /* The call to fork failed.  */
        system_error("fork");
    /* Write the end of the page.  */
    write(fd, page_end, strlen(page_end));
}
