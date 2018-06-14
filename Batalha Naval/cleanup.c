/* cleanup.c */

#include "bttlship.h"

/*
 * Cleanup procedure called by atexit processing:
 */
void
cleanup(void) {
    union semun semarg;

    /*
     * Detach shared memory if it is attached:
     */
    if ( shmp != 0 && shmp != (char *)(-1) )
        if ( shmdt(shmp) == -1 )
            perror("shmdt()");

    /*
     * Destroy shared memory:
     */
    if ( shmid != -1 )
        if ( shmctl(shmid,IPC_RMID,0) == -1 )
            perror("shmctl(IPC_RMID)");

    /*
     * Destroy semaphore:
     */
    if ( semid != -1 )
        if ( semctl(semid,0,IPC_RMID,semarg) == -1 )
            perror("semctl(IPC_RMID)");
}

/* End cleanup.c */
