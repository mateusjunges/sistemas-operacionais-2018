#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <wctype.h>
#include "server.h"

extern char* params;

static char* page_start = //conteudo a ser exibido no inicio da pagina
        "<html>\n"
        "<head>\n"
        "    <meta charset=\"utf-8\">\n"
        "</head>\n"
       " <body background='https://uploaddeimagens.com.br/images/001/459/455/original/logo_UEPG2.png?1528686439' background-repeat='no-repeat'>\n"
	"<div class='inicio'>"
		"<h1>Módulo de calendário - ALP - SO 2018</h1>\n"
	"</div>\n"
	"<br><br><br><br><br>\n"
	"<div>\n"
        "<pre id='calendario'>\n <!--tag pre pra elemento pre formatado, no caso vem do resultado do execv -->";


static char* page_end = //conteudo html a ser exibido no fim da pagina
        "  </pre>\n"
	"</div>"
	" <style>"
".footer {"
	"position: fixed;"
	"left: 0;"
	" bottom: 0;"
	"width: 100%;"
	"color: black;"
	"background: white;"	
	"text-align: center;"
	"text-decoration: none;"
"}"
" .footer > h3 {"
	"color: black;"
"}"
".footer > h4 {"
	"color: black;"
	"text-decoration: none;"
"}"
".inicio {"
	"position: fixed;"
	"top: 0;"
	"margin-botton: 20px;"
	"text-align: center;"
	"background: white;"
	"width: 100%;"
"}"
"#calendario {"
	"text-align: center;"
	"width: auto;"
"}"
"</style>"

"<br><br><br><br><br><br><br>"
"<div class='footer'>"
	"<h2>Universidade Estadual de Ponta Grossa - UEPG</h2>"
	"<h3>Sistemas Operacionais - Engenharia de Computação</h3>"
	"<h4>Desenvolvimento:<a href='https://mateusjunges.com'> Mateus Junges</a></h4>"
"</div> "
"</body>\n"
"<script>"
	"alert('Atividade da disciplina de Sistemas operacionais, do 3º ano de engenharia de computação, na Universidad Estadual de Ponta Grossa - Mateus Junges')"
"</script>"
"</html>\n";

void module_generate(int fd) {
    pid_t child_pid;
    int rval;


    write(fd, page_start, strlen(page_start)); //inicio da pagina html

    child_pid = fork();
    if (child_pid == 0) {
        char* ano = NULL; //ano
        char* mes = NULL; //mes
        int a, m, flag = 0; // inteiros para ano e mes
        char *aux = malloc(sizeof (params));
        strcpy(aux, params);
        if (strcmp(aux, "")) {
            mes = malloc(sizeof (aux));
            ano = malloc(sizeof (aux));
            strcpy(ano, aux);
            ano = strstr(ano, "ano");
            if (ano != NULL) { // se o parametro ano existe
                ano = strchr(ano, '=') + 1;
                strtok(ano, "&");
                a = atoi(ano);
            }
            strcpy(mes, aux);
            mes = strstr(mes, "mes");
            if (mes != NULL) { //se existe o parametro mes
                mes = strchr(mes, '=') + 1;
                m = atoi(mes); //passa para inteiro
                strtok(mes, "&");
            }
            if (strstr(aux, "&") != NULL) {
                flag = 1;
            }

        }


        rval = dup2(fd, STDOUT_FILENO);
        if (rval == -1)
            system_error("dup2");
        rval = dup2(fd, STDERR_FILENO);
        if (rval == -1)
            system_error("dup2");
 
        if (mes == NULL) { //se nao passou o parametro para o mes
            if (flag == 1) {
                write(fd, "<div style='background: red; margin: auto; border-radius: 10%; text-align: center; width: 80%;'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n", strlen("<div style='background: red; margin: auto; border-radius: 10%; text-align: center; width: 80%;'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n"));
            }
            if (ano == NULL) { // se o ano for Nulo, executa o comando cal com o parametro de help
                char* argv[] = {"/usr/bin/cal", "-h", NULL};
                execv(argv[0], argv);
            } else {
                if (a < 0 || a > 9999 || a == NULL) { //se for um ano não válido
                    write(fd, "<div style='background: red; text-align: center;'><h1><b>Parâmetros incorretos</b></h1>\n", strlen("<div style='background: red; text-align: center;'><h1><b>Parâmetros incorretos</b></h1>\n"));
                write(fd, "<br><h3><b>Ano deve ser um número entre 0 e 9999</b></h3>\n", strlen("<br><h3><b>Mês deve ser um número entre 1 e 12</b></h3></div>\n"));
                    char* argv[] = {"/usr/bin/cal", "-h", NULL};
                    execv(argv[0], argv);
                } else {
                    char* argv[] = {"/usr/bin/cal", ano, NULL};
                    execv(argv[0], argv);
                }
            }
        } else {
            if (m < 1 || m > 12 || m == NULL) { // se for um mes não válido
                write(fd, "<div style='background: red; text-align: center;'><h1><b>Parâmetros incorretos</b></h1>\n", strlen("<div style='background: red; text-align: center;'><h1><b>Parâmetros incorretos</b></h1>\n"));
                write(fd, "<br><h3><b>Mês deve ser um número entre 1 e 12</b></h3>\n", strlen("<br><h3><b>Mês deve ser um número entre 1 e 12</b></h3></div>\n"));
                char* argv[] = {"/usr/bin/cal", "-h", NULL};
                execv(argv[0], argv);
            } else {
                if (ano == NULL) {
                    if (flag == 1) {
                        write(fd, "<div style='background: red; margin: auto; text-align: center; width: 80%; border-radius: 10%'><h1>Os parâmetros corretos são: ano=ano&mes=mes</h1></div>\n\n", strlen("<div style='background:red; margin: auto; text-align: center; width: 80%; border-radius: 10%'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n"));
                    }
                    char* argv[] = {"/usr/bin/cal", "-m", mes, NULL};
                    execv(argv[0], argv);
                } else {
                    if (a < 0 || a > 9999 || a == NULL) { //se o ano existe mas não é valido
                        write(fd, "<div style='background: red;'><h1 style='text-align: center;><b>Formato incorreto</b></h1>\n", strlen("<div style='background: red;'><h1 style='text-align: center;><b>Formato incorreto</b></h1>\n"));
                        write(fd, "<br><b>Ano deve ser um número entre 0 e 9999</b></div>\n", strlen("<br><b>Ano deve ser um número entre 0 e 9999</b></div>\n"));
                        char* argv[] = {"/usr/bin/cal", "-h", NULL};  //executa cal com -h
                        execv(argv[0], argv);
                    } else {
			/* Se for ano e mes valido executa o 
			cal com os parametros ano e mes 
			passados na URL */
                        char* argv[] = {"/usr/bin/cal", mes, ano, NULL};
                        execv(argv[0], argv); 
                    }
                }
            }
        }

        system_error("execv"); // se erro
    } else if (child_pid > 0) {
        rval = waitpid(child_pid, NULL, 0);
        if (rval == -1)
            system_error("waitpid"); //se erro
    } else	
        system_error("fork");
    write(fd, page_end, strlen(page_end)); //final da pagina
}
