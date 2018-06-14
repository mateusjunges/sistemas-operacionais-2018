/* bttlship.h */
#ifndef _bttlship_h_
#define _bttlship_h_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define N_X         10      /* X dimension */
#define N_Y         10      /* Y dimension */
#define N_Z         5       /* Max length of ship */
#define N_SHIPS     6       /* Max number of ships */

#define INP_NONE    0       /* No input / not recognizable */
#define INP_YX      1       /* YX was given */

#define LOCK        lockTable(0,1)  /* Lock Table */
#define UNLOCK      lockTable(0,0)  /* Unlock Table */

#define WAIT2       lockTable(1,1)  /* Wait for Notify */
#define NOTIFY2     lockTable(1,0)  /* Notify player 1 */

/*
 * Table flags :
 */
#define FLG_P1      001     /* Owned by player 1 */
#define FLG_P2      002     /* Owned by player 2 */
#define FLG_SEEN0   004     /* Seen by player 1 */
#define FLG_SEEN1   010     /* Seen by player 2 */
#define FLG_BOMBD   020     /* Bombed */
#define FLG_SPLSH   040     /* Splash */

/*
 * Shared Memory Table :
 */
struct S_TABLE {
    int     semid;          /* Locking sem IPC ID */
    struct  {
      pid_t pid;            /* Process ID of player */
      int   bsites;         /* Sites left for bombing */
    }       player[2];
    char    sea[N_X][N_Y];  /* Matrix of sea locations */
    char    flg[N_X][N_Y];  /* Flags */
};

extern struct S_TABLE *table;

extern int shmid;           /* Shared Memory IPC ID */
extern int semid;           /* Table locking semaphore */
extern char *shmp;          /* Pointer to shared memory */
extern int us;              /* 0=starter / 1=challenger */
extern int them;            /* 1=challenger / 0=starter */
extern int flg_game_over;   /* != 0 => game over */

extern void cleanup(void);
extern void attachTable(void);
extern void lockTable(int semx,int bLock);

extern void recount(void);
extern void bomb(int x,int y);
extern int getInput(int *px,int *py);
extern int draw_hz(int sx,int sy,int z,int who);
extern int draw_vt(int sx,int sy,int z,int who);
extern void genBattle(void);
extern void showRow(void);
extern void showBattle(void);

#endif /* _bttlship_h_ */
