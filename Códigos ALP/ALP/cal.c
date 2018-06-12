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
            "<title>Calendário - SO</title>"
            "<meta charset=\"utf-8\">\n"
	    "</head>\n"
       " <body>\n"
	"<div class='inicio'>"
		"<h1>Módulo de calendário - ALP - SO 2018</h1>\n"
	"</div>\n"
	"<br><br><br><br><br>\n"
	"<div>\n"
        "<pre id='calendario'>\n <!--tag pre pra elemento pre formatado, no caso vem do resultado do execv -->";


static char* page_end = //conteudo html a ser exibido no fim da pagina
        "</pre>\n"
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
"pre {"
	"background: url('https://uploaddeimagens.com.br/images/001/459/455/original/logo_UEPG2.png?1528686439');"
	"background-position: center;"
	"background-repeat: no-repeat;"
	"background-size: 200px 200px;"
"}"
"</style>"

"<br><br><br><br><br><br><br>"
"<div class='footer' background='https://uploaddeimagens.com.br/images/001/459/455/original/logo_UEPG2.png?1528686439'>"
	"<h2>Universidade Estadual de Ponta Grossa - UEPG</h2>"
	"<h3>Sistemas Operacionais - Engenharia de Computação</h3>"
	"<div>"
		"<h4><a href='https://mateusjunges.com'"
			"style='text-decoration: none; color: #000000;'"
			"class=''>Mateus Junges</a></h4>"
		"<a href='mailto:contato@mateusjunges.com'>Email</a>"
 		"</div>"
	"</div> "
"</body>\n"
"<script>"
	"alert('Atividade da disciplina de Sistemas operacionais, do 3º ano de engenharia de computação, na Universidad Estadual de Ponta Grossa - Mateus Junges')"
"</script>"
"</html>\n";


static char* erro =
		"<div style='background: red;\n"
		     " margin: auto;\n"
		     "border-radius: 10%;\n"
		     "text-align: center;\n"
		     "width: 80%;'>\n"
		     "<h1>Os parâmetros corretos são ano=ano&mes=mes</h1>\n"
               "</div>\n";

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
                write(fd, "<div style='background: red;\n"
		     " margin: auto;\n"
		     "border-radius: 10%;\n"
		     "text-align: center;\n"
		     "width: 80%;'>\n"
		     "<h1>Os parâmetros corretos são ano=ano&mes=mes</h1>\n"
               "</div>\n", strlen("<div style='background: red;\n"
		     " margin: auto;\n"
		     "border-radius: 10%;\n"
		     "text-align: center;\n"
		     "width: 80%;'>\n"
		     "<h1>Os parâmetros corretos são ano=ano&mes=mes</h1>\n"
               "</div>\n"));
            }
            if (ano == NULL) { // se o ano for NULL e mes for NULL
                char* argv[] = {"/usr/bin/cal", NULL, NULL}; //Se passar só o cal na URL, busca o mes corrente do ano corrente
                execv(argv[0], argv);
            } else { // Se o mes é NULL e o ano existe
                if (a < 0 || a > 9999 || a == NULL) { //se for um ano não válido
                    write(fd, "<div style='background: red; text-align: center;'><h1><b>Parâmetros incorretos</b></h1>\n", strlen("<div style='background: red; text-align: center;'><h1><b>Parâmetros incorretos</b></h1>\n"));
                    write(fd, "<br><h3><b>Ano deve ser um número entre 0 e 9999</b></h3>\n", strlen("<br><h3><b>Mês deve ser um número entre 1 e 12</b></h3></div>\n"));
                    char* argv[] = {"/usr/bin/cal", "-h", NULL}; //Executa o cal com o parametro '-h'
                    execv(argv[0], argv);
                } else {
                    char* argv[] = {"/usr/bin/cal", ano, NULL};
                    execv(argv[0], argv);
                }
            }
        } else { //se o mes não for NULL
            if (m < 1 || m > 12 || m == NULL) { // se for um mes não válido
                write(fd, "<div style='background: red; text-align: center;'><h1><b>Parâmetros incorretos</b></h1>\n", strlen("<div style='background: red; text-align: center;'><h1><b>Parâmetros incorretos</b></h1>\n"));
                write(fd, "<br><h3><b>Mês deve ser um número entre 1 e 12</b></h3>\n", strlen("<br><h3><b>Mês deve ser um número entre 1 e 12</b></h3></div>\n"));
                char* argv[] = {"/usr/bin/cal", "-h", NULL};
                execv(argv[0], argv);
            } else {
                if (ano == NULL) {//se for um mês válido mas o ano está NULL
                    if (flag == 1) {
                        write(fd, "<div style='background: red; margin: auto; text-align: center; width: 80%; border-radius: 10%'><h1>Os parâmetros corretos são: ano=ano&mes=mes</h1></div>\n\n", strlen("<div style='background:red; margin: auto; text-align: center; width: 80%; border-radius: 10%'><h1>Os parâmetros corretos são ano=ano&mes=mes</h1></div>\n"));
                    }
                    char* argv[] = {"/usr/bin/cal", "-m", mes, NULL};
                    execv(argv[0], argv); //Executa o comando cal com o parametro -m e o mes passado na URL
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
                        execv(argv[0], argv); //Executa o cal com o ano e o mes passados na URL
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
