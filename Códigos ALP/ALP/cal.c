#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <wctype.h>

#include "server.h"

extern char* params;

static char* page_start =
        "<html>\n"
        "<head>\n"
        "    <meta charset=\"utf-8\">\n"
        "</head>\n"
        " <body>\n"
        "  <pre>\n";


static char* page_end =
        "  </pre>\n"
        " </body>\n"
        "</html>\n";

void module_generate(int fd) {
    pid_t child_pid;
    int rval;


    write(fd, page_start, strlen(page_start));

    child_pid = fork();
    if (child_pid == 0) {
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
            if (strcmp(aux, "&") != NULL) {
                flag = 1;
            }

        }


        rval = dup2(fd, STDOUT_FILENO);
        if (rval == -1)
            system_error("dup2");
        rval = dup2(fd, STDERR_FILENO);
        if (rval == -1)
            system_error("dup2");
 
        if (mes == NULL) {
            if (flag == 1) {
                write(fd, "<div style='background: red; margin: auto; border-radius: 10%; text-align: center; width: 80%;'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n", strlen("<div style='background: red; margin: auto; border-radius: 10%; text-align: center; width: 80%;'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n"));
            }
            if (ano == NULL) {
                char* argv[] = {"/usr/bin/cal", "-h", NULL};
                execv(argv[0], argv);
            } else {
                if (a < 0 || a > 9999 || a == NULL) {
                    write(fd, "<div style='background: red;'><h1 style='text-align: center;>Formato incorreto</b></h1>\n", strlen("<div style='background: red;'><h1 style='text-align: center;>Formato incorreto</b></h1>\n"));
                    write(fd, "<br><b>Ano deve ser um número entre 0 e 9999</b></div>\n", strlen("<br><b>Ano deve ser um número entre 0 e 9999</div>\n"));
                    char* argv[] = {"/usr/bin/cal", "-h", NULL};
                    execv(argv[0], argv);
                } else {
                    char* argv[] = {"/usr/bin/cal", "-h", ano, NULL};
                    execv(argv[0], argv);
                }
            }
        } else {
            if (m < 1 || m > 12 || m == NULL) {
                write(fd, "<div style='background: red; text-align: center;'><h1><b>Formato incorreto</b></h1>\n", strlen("<div style='background: red; text-align: center;'><h1><b>Formato incorreto</b></h1>\n"));
                write(fd, "<br><h3><b>Mês deve ser um número entre 1 e 12</b></h3>\n", strlen("<br><h3><b>Mês deve ser um número entre 1 e 12</b></h3></div>\n"));
                char* argv[] = {"/usr/bin/cal", "-h", NULL};
                execv(argv[0], argv);
            } else {
                if (ano == NULL) {
                    if (flag == 1) {
                        write(fd, "<div style='background: red; margin: auto; text-align: center; width: 80%; border-radius: 10%'><h1>Os parâmetros corretos são: ano=ano&mes=mes</h1></div>\n\n", strlen("<div style='background:red; margin: auto; text-align: center; width: 80%; border-radius: 10%'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n"));
                    }
                    char* argv[] = {"/usr/bin/cal", "-h", "-m", mes, NULL};
                    execv(argv[0], argv);
                } else {
                    if (a < 0 || a > 9999 || a == NULL) {
                        write(fd, "<div style='background: red;'><h1 style='text-align: center;><b>Formato incorreto</b></h1>\n", strlen("<div style='background: red;'><h1 style='text-align: center;><b>Formato incorreto</b></h1>\n"));
                        write(fd, "<br><b>Ano deve ser um número entre 0 e 9999</b></div>\n", strlen("<br><b>Ano deve ser um número entre 0 e 9999</b></div>\n"));
                        char* argv[] = {"/usr/bin/cal", "-h", NULL};
                        execv(argv[0], argv);
                    } else {
                        char* argv[] = {"/usr/bin/cal", "-h", mes, ano, NULL};
                        execv(argv[0], argv);
                    }
                }
            }
        }


        system_error("execv");
    } else if (child_pid > 0) {

        rval = waitpid(child_pid, NULL, 0);
        if (rval == -1)
            system_error("waitpid");
    } else	

        system_error("fork");

    write(fd, page_end, strlen(page_end));
}
