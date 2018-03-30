/*-------------------------------------------------------------------------*/
/*    A Multitasking Kernel for DOS                                        */
/*                                                                         */
/*    Do livro Born to Code in C                                           */
/*                                                                         */
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <alloc.h>
#include "mkern0.h"

struct task_struct
{
  unsigned sp;
  unsigned ss;
  enum task_state status;	/* task state */
  unsigned *pending;		/* semaphore waiting for */
  int sleep;			/* number of ticks to sleep */
  unsigned char *stck;		/* stack */
} tasks[NUM_TASKS];

void interrupt (*old_int8) (void);

unsigned io_out= 0;	/* i/o semaphore */
char far *vid_mem;	/* pointer to video memory */


/*-------------------------------------------------------------------------*/
/*  Timer interrupt task scheduler                                         */
/*-------------------------------------------------------------------------*/
void interrupt int8_task_switch(void)
{
  (*old_int8) ();		/* call original int8 routine */
  if(single_task)		/* if single tasking is on, then return  */
    return;			/* without a task switch */
  tasks[tswitch].ss = _SS;	/* save current task's stack */
  tasks[tswitch].sp = _SP;

  /* if current task was running when interrupted, then change
     its state to READY */
  if(tasks[tswitch].status == RUNNING)
    tasks[tswitch].status = READY;

  /* see if any sleepers need to wake up */
  check_sleepers();

  /* see if all tasks are dead; if so, stop tasking */
  if(all_dead())
    tasking = 0;

  /* stop tasking */
  if(!tasking)
  {
     disable();
       _SS = oldss;
       _SP = oldsp;
       setvect(8, old_int8);
       free_all();
     enable();
     return;
  }

  /* find new task */
  tswitch++;
  if(tswitch == NUM_TASKS)
    tswitch = 0;
  while(tasks[tswitch].status != READY)
  {
    tswitch++;
    if(tswitch == NUM_TASKS)
      tswitch = 0;
  }
   _SS = tasks[tswitch].ss;		/* switch task to new task */
   _SP = tasks[tswitch].sp;
  tasks[tswitch].status = RUNNING;     	/* state is running */
}


/*-------------------------------------------------------------------------*/
/*   task_switch()                                                         */
/*                                                                         */
/*   This is the manual task switcher which your program can call to force */
/* a task switch. It does not decrement any sleeper's sleep counter        */
/* because a clock tick has not occurred.                                  */
/*-------------------------------------------------------------------------*/
void interrupt task_switch(void)
{
  if(single_task)
    return;

  disable();
    tasks[tswitch].ss = _SS;	/* save current task's stack */
    tasks[tswitch].sp = _SP;

    /* if current task was running when interrupted, then change
     its state to READY */
    if(tasks[tswitch].status == RUNNING)
      tasks[tswitch].status = READY;

    /* see if all tasks are dead; if so, stop tasking */
    if(all_dead())
      tasking = 0;

    /* stop tasking */
    if(!tasking)
    {
      disable();
	_SS = oldss;
	_SP = oldsp;
	setvect(8, old_int8);
	free_all();
      enable();
      return;
    }

    /* find new task */
    tswitch++;
    if(tswitch == NUM_TASKS)
      tswitch = 0;
    while(tasks[tswitch].status != READY)
    {
      tswitch++;
      if(tswitch == NUM_TASKS)
	tswitch = 0;
    }
    _SS = tasks[tswitch].ss;		/* switch task to new task */
    _SP = tasks[tswitch].sp;
    tasks[tswitch].status = RUNNING;   	/* state is running */
  enable();
}


/*-------------------------------------------------------------------------*/
/*   multitask()                                                           */
/*                                                                         */
/*   Start up the multitasking kernel.                                     */
/*-------------------------------------------------------------------------*/
void interrupt multitask(void)
{
  disable();

    /* switch in the timer-based scheduler */
    old_int8 = getvect(8);
    setvect(8, int8_task_switch);

    /* save the program's stack pointer and segment so that when tasking
       ends, execution can continue where it left off in the program.
    */
    oldss = _SS;
    oldsp = _SP;

    /* set stack to first task's stack */
    _SS = tasks[tswitch].ss;
    _SP = tasks[tswitch].sp;
  enable();
}


/*-------------------------------------------------------------------------*/
/*   make_task()                                                           */
/*                                                                         */
/*   Returns false if the task cannot be added to the task queue.          */
/* Otherwise, it returns true.                                             */
/*-------------------------------------------------------------------------*/
int make_task(taskptr task,
	      unsigned stck,
	      unsigned id)
{
  struct int_regs *r;

  if((id>=NUM_TASKS) || (id<0))
    return 0;

  disable();
    /* allocate space for the task */
    tasks[id].stck = malloc(stck + sizeof(struct int_regs));
    r = (struct int_regs *) tasks[id].stck + stck -  sizeof(struct int_regs);

    /* initialize task stack */
    tasks[id].sp = FP_OFF((struct int_regs far *) r);
    tasks[id].ss = FP_SEG((struct int_regs far *) r);

    /* set up new task's CS and IP registers */
    r->cs = FP_SEG(task);
    r->ip = FP_OFF(task);

    /* set up DS and ES */
    r->ds = _DS;
    r->es = _DS;

    /* enable interrupts */
    r->flags = 0x200;

    tasks[id].status = READY;
  enable();
  return 1;
}


/*-------------------------------------------------------------------------*/
/*   free_all()                                                            */
/*                                                                         */
/*   Free all stack space. This function should not be called by your      */
/* program                                                                 */
/*-------------------------------------------------------------------------*/
void free_all(void)
{
  register int i;

  for(i=0; i<NUM_TASKS; i++)
  {
    if(tasks[i].stck)
    {
      free(tasks[i].stck);
      tasks[i].stck = NULL;
    }
  }
}


/*-------------------------------------------------------------------------*/
/*  kill_task()                                                            */
/*                                                                         */
/*  Kill a task (make its state DEAD)                                      */
/*-------------------------------------------------------------------------*/
void kill_task(int id)
{
  disable();
    tasks[id].status = DEAD;
    free(tasks[id].stck);
    tasks[id].stck = NULL;
  enable();
  task_switch();
}

/*-------------------------------------------------------------------------*/
/*   init_tasks()                                                          */
/*                                                                         */
/*   Initialize the task control structures                                */
/*-------------------------------------------------------------------------*/
void init_tasks(void)
{
  register int i;

  for(i=0; i<NUM_TASKS; i++)
  {
    tasks[i].status  = DEAD;
    tasks[i].pending = NULL;
    tasks[i].sleep   = 0;
    tasks[i].stck    = NULL;
  }
  set_vid_mem();
}

/*-------------------------------------------------------------------------*/
/*   stop_tasking()                                                        */
/*                                                                         */
/*   Stop tasking.                                                         */
/*-------------------------------------------------------------------------*/
void stop_tasking(void)
{
  tasking = 0;
  task_switch();
}

/*-------------------------------------------------------------------------*/
/*   mono_task()                                                           */
/*                                                                         */
/*   Execute only one task.                                                */
/*-------------------------------------------------------------------------*/
void mono_task(void)
{
  disable();
    single_task = 1;
  enable();
}

/*-------------------------------------------------------------------------*/
/*   resume_tasking()                                                      */
/*                                                                         */
/*   Resume multitasking all tasks. (Use to restart tasking after a call   */
/* to mono_task().                                                         */
/*-------------------------------------------------------------------------*/
void resume_tasking(void)
{
  single_task = 0;
}


/*-------------------------------------------------------------------------*/
/*   all_dead(void)                                                        */
/*                                                                         */
/*  Return 1 if no tasks are ready to run; 0 if at least one task is READY */
/*-------------------------------------------------------------------------*/
int all_dead(void)
{
  register int i;

  for(i=0; i<NUM_TASKS; i++)
    if(tasks[i].status == READY)
      return 0;
  return 1;
}


/*-------------------------------------------------------------------------*/
/*   check_sleepers()                                                      */
/*                                                                         */
/*   Decrement the sleep count of any sleeping tasks                       */
/*-------------------------------------------------------------------------*/
void check_sleepers(void)
{
  register int i;

  for(i=0; i<NUM_TASKS; i++)
  {
    if(tasks[i].status == SLEEPING)
    {
      tasks[i].sleep--;
      if(!tasks[i].sleep)
	tasks[i].status = READY;
    }
  }
}


/*-------------------------------------------------------------------------*/
/*   msleep()                                                              */
/*                                                                         */
/*   Stop execution of a task for a specified number of clock cycles.      */
/*-------------------------------------------------------------------------*/
void msleep(int ticks)
{
  disable();
    tasks[tswitch].status = SLEEPING;
    tasks[tswitch].sleep  = ticks;
  enable();
  task_switch();
}


/*-------------------------------------------------------------------------*/
/*   suspend()                                                             */
/*                                                                         */
/*   Suspend a task until resumed by another task                          */
/*-------------------------------------------------------------------------*/
void suspend(int id)
{
  if(id<0 || id>=NUM_TASKS)
    return;
  tasks[id].status = SUSPENDED;
  task_switch();
}


/*-------------------------------------------------------------------------*/
/*   resume()                                                              */
/*                                                                         */
/*   Restart a previously suspended task.                                  */
/*-------------------------------------------------------------------------*/
void resume(int id)
{
  if(id<0 || id>=NUM_TASKS)
    return;
  tasks[id].status = READY;
}


/*-------------------------------------------------------------------------*/
/*   set_semaphore                                                         */
/*                                                                         */
/*   Wait for a semaphore.                                                 */
/*-------------------------------------------------------------------------*/
void set_semaphore(unsigned *sem)
{
  disable();
    while(*sem)
    {
      semblock(tswitch, sem);
      task_switch();
      disable();	/* task switch will enable interrupts, so they need
			   to be turned off again */
    }
    *sem = 1;
  enable();
}


/*-------------------------------------------------------------------------*/
/*   clear_semaphore                                                       */
/*                                                                         */
/*   Release a semaphore.                                                  */
/*-------------------------------------------------------------------------*/
void clear_semaphore(unsigned *sem)
{
  disable();
    tasks[tswitch].pending = NULL;
    *sem = 0;
    restart(sem);
    task_switch();
  enable();
}


/*-------------------------------------------------------------------------*/
/*   semblock()                                                            */
/*                                                                         */
/*   Set task to BLOCKED. This is an internal function not to be called by */
/* your program.                                                           */
/*-------------------------------------------------------------------------*/
void semblock(int id, unsigned *sem)
{
  tasks[id].status  = BLOCKED;
  tasks[id].pending = sem;
}


/*-------------------------------------------------------------------------*/
/*   restart()                                                             */
/*                                                                         */
/*   Restart a task that is waiting for the specified semaphore. This is   */
/* an internal function not to be called by your program.                  */
/*-------------------------------------------------------------------------*/
void restart(unsigned *sem)
{
  register int i;

  for(i=0; i<NUM_TASKS; i++)
    if(tasks[i].pending == sem)
    {
      tasks[i].pending = NULL;
      if(tasks[i].status == BLOCKED)
	tasks[i].status = READY;
      return;
    }
}


/*-------------------------------------------------------------------------*/
/*   task_status()                                                         */
/*                                                                         */
/*   Display the state of all tasks. This function must NOT be called      */
/* while multitasking is in effect.                                        */
/*-------------------------------------------------------------------------*/
void task_status(void)
{
  register int i;

  if(tasking)	/* cannot be used while multitasking */
    return;

  printf("\n");

  for(i=0; i<NUM_TASKS; i++)
  {
    printf("Task %d: ",i);

    switch(tasks[i].status)
    {
      case READY:
	printf("READY\n");
	break;
      case RUNNING:
	printf("RUNNING\n");
	break;
      case BLOCKED:
	printf("BLOCKED\n");
	break;
      case SUSPENDED:
	printf("SUSPENDED\n");
	break;
      case SLEEPING:
	printf("SLEEPING\n");
	break;
      case DEAD:
	printf("DEAD\n");
	break;
    }
  }
}


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*	Serialized and Reentrant I/O functions for the Multitasker         */
/*                                                                         */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* void mputs(char *s)                                                     */
/*                                                                         */
/*   Serialized version of puts()                                          */
/*-------------------------------------------------------------------------*/
void mputs(char *s)
{
  set_semaphore(&io_out);
  puts(s);
  clear_semaphore(&io_out);
}


/*-------------------------------------------------------------------------*/
/* 	Output a number                                                    */
/*-------------------------------------------------------------------------*/
void mputnum(int num)
{
  set_semaphore(&io_out);
  printf("%d", num);
  clear_semaphore(&io_out);
}


/*-------------------------------------------------------------------------*/
/* 	Serialized version of getche()                                     */
/*-------------------------------------------------------------------------*/
char mgetche(void)
{
  char ch;

  set_semaphore(&io_out);
  ch= getch();
  clear_semaphore(&io_out);
  return ch;
}


/*-------------------------------------------------------------------------*/
/* void mxyputs()                                                          */
/*                                                                         */
/*   Output a string at specified X, Y coordinates. This function is       */
/* reentrant and may be called by any task at any time.                    */
/*-------------------------------------------------------------------------*/
void mxyputs(int x, int y, char *str)
{
  while(*str)
    moutchar(x++,y,*str++);
}

/*-------------------------------------------------------------------------*/
/* void moutchar()                                                         */
/*                                                                         */
/*   Output a character at specified X, Y coordinates. This function is    */
/* reentrant and may be called by any task at any time.                    */
/*-------------------------------------------------------------------------*/
void moutchar(int x, int y, char ch)
{
  char far *v;

  v = vid_mem;

  v += (y*160) + x*2;	/* compute char loc */
  *v++ = ch;		/* write the character */
  *v = 7;		/* normal character */
}

/*-------------------------------------------------------------------------*/
/*	Initialize the video subsystem                                     */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* void video_mode()                                                       */
/*                                                                         */
/*   Return the current video mode                                         */
/*-------------------------------------------------------------------------*/
video_mode(void)
{
  union REGS r;

  r.h.ah = 15;	/* get video mode */
  return int86( 0x10, &r, &r) & 255;
}

/*-------------------------------------------------------------------------*/
/* void set_vid_mem()                                                      */
/*                                                                         */
/*   Set the vid_mem pointer to the start of video memory                  */
/*-------------------------------------------------------------------------*/
void set_vid_mem(void)
{
  int vmode;

  vmode= video_mode();
  if((vmode!=2) && (vmode!=3) && (vmode!=7))
  {
    printf("/n Video must be in 80 column text mode");
    exit(1);
  }

  /* set proper address of video RAM */
  if(vmode==7)
    vid_mem = (char far *) MK_FP(0xB000, 0);
  else
    vid_mem = (char far *) MK_FP(0xB800, 0);
}
