/* semop.c */

#include "bttlship.h"

/*
 * Peform semaphore wait/notifies:
 * ARGUMENTS:
 *  semx    0 : table lock semaphore
 *          1 : opponent notify semaphore
 *  bLock   0 : perform notify
 *          1 : perform wait
 */
void
lockTable(int semx,int bLock) {
    int z;                          /* Return status */
    static struct sembuf sops = { 0, -1, 0 };

    /*
     * Lock or unlock the semaphore
     */
    sops.sem_num = semx;            /* Select semaphore */
    sops.sem_op = bLock ? -1 : 1;   /* Wait / Notify */
    do  {
        z = semop(semid,&sops,1);   /* Semaphore operation */
    } while ( z == -1 && errno == EINTR );

    if ( z == -1 ) {
        perror("semop()");          /* Should not happen */
        exit(13);
    }
}

/* End semop.c */
