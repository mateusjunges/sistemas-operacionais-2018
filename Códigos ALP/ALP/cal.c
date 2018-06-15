#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server.h"

		
void* parametros; //Parametros recebidos no GET

static char* page_start = //conteudo a ser exibido no inicio da pagina
        "<html>\n"
        "<head>\n"
            "<title>Calendário - SO</title>"
            "<meta charset=\"utf-8\">\n"
	    "<link rel='icon' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAOtSURBVGhD7ZpryE1ZGMdf9ztFxoxrNIhiJEL5ME2KSI2kqVHmg2sikXxjTFHCByQaxhcyhcTUpEbyxWUyJbfkUkMu4zK+MGWM6/j9tnd1drvjfc9hX6Tzr1/nrKe991rP2evyrPWcuvfUEngEY6LSu2kT/A09olJBWgb/w1Vop6FKTQTvfwzdNBSlFnAGbMxGDVWoM/wF3rtIQ9EaBs/gFXwDM+FnuAj34SX8Czfhd1gPE8BrdOIYNIUPQtvBRpVDR8rZAyOhcLWHbRAa+x8cgjkwFOz3TaA19IEvYQ1cguCIb20aFKa+EBqkIzvgM6hUdi+7X3DIMabTucpf9w7YgHsQpl8Hf6Xy2uawGoIzP0FuspucAyu+Bp+Dg/UwvICF0JBawSl4Ct9qQPMgdE/Xply0DqzQmWiwBvQphF/1pIYG9AWEaw9qqJdjR5vjbJCGLNUTrMgKF2iIyT5+Bb6OSm9XM9gJl8HBH9QSToPP3qchS/0AVuT4sH+nLX8En283664hK10AK9oC/TJgADwA65gPmagNNLa4pYndL1V1gbbQCazgOfyZIXfBen4F1RWcKd9Zrg1hmpXj9Z83IEtNAuu5BSGodKo2gugIVWkEGAj6EPcbhtl+z9ORgCGM65PfT4CzXsVyLfBGI1UXO8eHU6K2vByx8WEqd9p3utb+nYZK5LTqoHYsxPumi1Sejhjex+Uspt1IuyKFQf0wKpUU7Hk5EgZ7kOGM9t1RqQLVHElJNUeSqjmSkmqOJFVzJCXVHEnqo3GkA3jDP1GpJPclRToyHapyRBm6e1P/qPRG4eQ8L0euQ/ycbC1o97NimbPwJk8CfaWzIezc8nJEfoEpsBLcXBmRe2hesTzT/QPCA+Pk6Ugc9yeLoWq5L3EPsB92wSzwgXk54uZuBRwAt7mjIBUVPWulpuDI7aiUnSaD9Tg+MpNJSzNTS8GcRhaYX9GRVZCZ5oKOWFGWmKrwYDxTjYMfwT5spR4V7Y3xG2g3Mojbj4J2j0TL2T1P3gO+iVwzvL3BBiQH/xDQfj4qlTQWtCdPR4I9s8HdmGqORKWSao6kpY/GkV5gAzw1j8ugTvvZqFTSaNDuvx/i+goKdcTsrPlEgzlz7kHmFm1YcmU2leYa5D7HvU2Q063Xb41KBWkz2AjT1K72nt4/ARs8HpIyAPV6U3imoU3hGZrLcChMhvtHwMYFfEPLoZw+geT2wLdqxFC4/NvFVNgAdpPGflm3BzPAzdv3MBDeU3V1rwE/FuMlAfXKBQAAAABJRU5ErkJggg=='>"	
	    "</head>\n"
       " <body>\n"
	"<div class='inicio'>"
		"<h1>Módulo de calendário - ALP - SO 2018</h1>\n"
	"</div>\n"
	"<br><br><br><br><br>\n"
	"<div class='text'></div>\n"
        "<pre id='calendario'>\n <!--tag pre pra elemento pre formatado, no caso vem do resultado do execv -->";


static char* page_end = //conteudo html a ser exibido no fim da pagina
        "</pre>\n"
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
"body {"
	"background: url('https://uploaddeimagens.com.br/images/001/459/455/original/logo_UEPG2.png?1528686439');"
	"background-position: center center;"
	"background-repeat: no-repeat;"
	"background-size: 300px 300px;"
	"background-attachment: fixed;"
"}"
".text {"
	"opacity: 1.0;"
"}"
"pre {"
    "min-width: 200px;"
    "min-height: 200px;"
"}"
"</style>"

"<br><br><br><br><br><br><br>"
"<div class='footer'>"
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
"<script src='https://unpkg.com/sweetalert2@7.17.0/dist/sweetalert2.all.js'></script>"
"\n<script type='text/javascript'>\n"
	"swal({"
		"icon: 'warning',"
		"title: 'Sistemas Operacionais - Calendário',"
		"html: 'Utilize os parâmetros: <br><b>ano=XXXX</b>, para retornar o calendario do ano desejado, <br><b>mes=XX</b>, para retornar o calendário do mes desejado no ano corrente. <br>Da seguinte forma: <br><b>localhost:4444/cal?ano=XXXX&mes=XX</b>, <br>ou <b>localhost:4444/cal?ano=XXXX</b>, <br><b>ou localhost:4444/cal?mes=XX</b>'," 	
	"});"
"</script>"

"</html>\n";
void module_generate (int fd)
{
  pid_t child_pid;
  int rval;

  /* Write the start of the page.  */
  write (fd, page_start, strlen (page_start));
  /* Fork a child process.  */
  child_pid = fork ();
  if (child_pid == 0) {
    /* This is the child process.  */
    /* Set up an argumnet list for the invocation of df.  */
    /* Duplicate stdout and stderr to send data to the client socket.  */
    rval = dup2 (fd, STDOUT_FILENO);
    if (rval == -1)
      system_error ("dup2");
    rval = dup2 (fd, STDERR_FILENO);
    if (rval == -1)
      system_error ("dup2");

          if(parametros == NULL){
            char* argv[] = { "/usr/bin/cal", "-h", NULL };
            execv (argv[0], argv);
          }else{
            char* aux_parametros = strtok(parametros, "&");
            char* aux_parametros2 = strtok(NULL, "&");
            int ano = -1, mes = -1;
            char* param1 = strtok(aux_parametros, "=");
            char* valor1 = strtok(NULL, "=");
            if(param1 == NULL || valor1 == NULL){
              write(fd, "\n Parametros invalidos! \n Uso: cal?ano=2000&mes=5\n\n", strlen("\n Parametros invalidos! \n Uso: cal?ano=2000&mes=5\n\n"));
            }
            char* param2 = "-", valor2 = "-";
            if(aux_parametros2 != NULL){
              char* param2 = strtok(aux_parametros2, "=");
              char* valor2 = strtok(NULL, "=");
              if(param2 == NULL || valor2 == NULL){
                write(fd, "\n Parametros invalidos! \n Uso: cal?ano=2000&mes=5\n\n", strlen("\n Parametros invalidos! \n Uso: cal?ano=2000&mes=5\n\n"));
              }
              if(!strcmp(param1, "ano") && !strcmp(param2, "mes")){
                int ano = atoi(valor1);
                int mes = atoi(valor2);
                if(ano < 1 || ano > 9999){
                  write(fd, "\n\nO ano deve ser entre 0 e 9999\n\n", strlen("\n\nO ano deve ser entre 0 e 9999\n\n"));
                  char* argv[] = { "/usr/bin/cal", "-h", NULL };
                  execv (argv[0], argv);
                }else if(mes < 1 || mes > 12){
                  write(fd, "\n\nO mes deve ser entre 1 e 12\n\n", strlen("\n\nO mes deve ser entre 1 e 12\n\n"));
                  char* argv[] = { "/usr/bin/cal", "-h", NULL };
                  execv (argv[0], argv);
                }else{
                  char* argv[] = { "/usr/bin/cal", valor2, valor1, NULL };
                  execv (argv[0], argv);
                }
              }else if(!strcmp(param1, "mes") && !strcmp(param2, "ano")){
                int ano = atoi(valor2);
                int mes = atoi(valor1);
                if(ano < 1 || ano > 9999){
                  write(fd, "\n\nO ano deve ser entre 0 e 9999\n\n", strlen("\n\nO ano deve ser entre 0 e 9999\n\n"));
                  char* argv[] = { "/usr/bin/cal", "-h", NULL };
                  execv (argv[0], argv);
                }else if(mes < 1 || mes > 12){
                  write(fd, "\n\nO mes deve ser entre 1 e 12\n\n", strlen("\n\nO mes deve ser entre 1 e 12\n\n"));
                  char* argv[] = { "/usr/bin/cal", "-h", NULL };
                  execv (argv[0], argv);
                }else{
                  char* argv[] = { "/usr/bin/cal", valor1, valor2, NULL };
                  execv (argv[0], argv);
                }
              }else{
                write(fd, "\n Parametros invalidos! \n Uso: cal?ano=2000&mes=5\n\n", strlen("\n Parametros invalidos! \n Uso: cal?ano=2000&mes=5\n\n"));
                char* argv[] = { "/usr/bin/cal", "-h", NULL };
                execv (argv[0], argv);
              }
            }else{
              if(!strcmp(param1, "ano")){
                int ano = atoi(valor1);
                if(ano < 1 || ano > 9999){
                  write(fd, "\n\nO ano deve ser entre 0 e 9999\n\n", strlen("\n\nO ano deve ser entre 0 e 9999\n\n"));
                  char* argv[] = { "/usr/bin/cal", "-h", NULL };
                  execv (argv[0], argv);
                }else{
                  char* argv[] = { "/usr/bin/cal", valor1, NULL };
                  execv (argv[0], argv);
                }
              }else if(!strcmp(param1, "mes")){
                int mes = atoi(valor1);
                if(mes < 1 || mes > 12){
                  write(fd, "\n\nO mes deve ser entre 1 e 12\n\n", strlen("\n\nO mes deve ser entre 1 e 12\n\n"));
                  char* argv[] = { "/usr/bin/cal", "-h", NULL };
                  execv (argv[0], argv);
                }else{
                  char* argv[] = { "/usr/bin/cal", "-m", valor1, NULL };
                  execv (argv[0], argv);
                }
              }else{
                write(fd, "\n Parametros invalidos! \n Uso: cal?ano=2000&mes=5\n\n", strlen("\n Parametros invalidos! \n Uso: cal?ano=2000&mes=5\n\n"));
                char* argv[] = { "/usr/bin/cal", "-h", NULL };
                execv (argv[0], argv);
              }
            }
          }
    /* Run df to show the free space on mounted file systems.  */
    //execv (argv[0], argv);
    /* A call to execv does not return unless an error occurred.  */
    system_error ("execv");
  }
  else if (child_pid > 0) {
    /* This is the parent process.  Wait for the child process to
       finish.  */
    rval = waitpid (child_pid, NULL, 0);
    if (rval == -1)
      system_error ("waitpid");
  }
  else
    /* The call to fork failed.  */
    system_error ("fork");
  /* Write the end of the page.  */
  write (fd, page_end, strlen (page_end));
}
