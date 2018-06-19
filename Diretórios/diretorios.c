#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void pesquisa(const char *arg)
{
    DIR *dirpont;
    struct dirent *dp;

    if ((dirpont = opendir( "." )) == NULL) {
        perror( "nao pode abrir '.'\n\n" );
        return;
    }

    do {
        errno = 0;
        if ((dp = readdir(dirpont)) != NULL) {
            if (strcmp(dp->d_name, arg) != 0)
                continue;
            (void) printf("achou: %s(%d)\n", dp->d_name, dp->d_type);
            (void) closedir(dirpont);
            return;
        }
    } while (dp != NULL);

    if (errno != 0)
        perror("Erro lendo o diretorio\n\n");
    else
        (void) printf("Falha ao procurar: %s\n", arg);
    (void) closedir(dirpont);
    return;
}

int main(int argc, char *argv[])
{
    int i;

    if( argc < 2 ) {
       printf( "USO: %s nome_arquivo ...\n\n", argv[ 0 ] );
       return( -1 );
    }    
    for (i = 1; i < argc; i++)
        pesquisa( argv[i] );
    return (0);
}