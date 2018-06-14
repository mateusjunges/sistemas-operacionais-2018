/* attch.c */

#include "bttlship.h"

void
attachTable(void) {

    /*
     * Attach the shared memory :
     */
    shmp = shmat(shmid,0,0);
    if ( shmp == (char *)(-1) ) {
        perror("shmat()");
        exit(13);
    }
    table = (struct S_TABLE *)shmp;
}

/* End attch.c */
