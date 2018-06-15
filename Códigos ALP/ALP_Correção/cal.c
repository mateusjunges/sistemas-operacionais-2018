#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server.h"

/*Parametros recebidos no GET*/
void* parametros;

/* HTML source for the start of the page we generate.  */

static char* page_start =
  "<html>\n"
  "<head>\n"
  "<title>Calendario</title>\n"
  "<meta name='charset' content='utf-8'>\n"
  "<head/>\n"
  " <body>\n"
  "  <pre>\n";

/* HTML source for the end of the page we generate.  */

static char* page_end =
  "  </pre>\n"
  " </body>\n"
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
