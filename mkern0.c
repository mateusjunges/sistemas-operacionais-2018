/*-------------------------------------------------------------------------*/
/*   Um kernel multitarefa para DOS                                    */
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
  enum task_state status;	/*Estado da tarefa*/
  unsigned *pending;		/*semaforo esperando por...*/
  int sleep;			/*tempo para sleep*/
  unsigned char *stck;		 /*pilha*/
} tasks[NUM_TASKS];

void interrupt (*old_int8) (void);

unsigned io_out= 0;	 /* semaforo para IO, entrada e saida*/
char far *vid_mem;	/*Ponteiro para memoria de video*/


/*-------------------------------------------------------------------------*/
/*  Timer de interrupção de tarefas dos escalonador                                         */
/*-------------------------------------------------------------------------*/
void interrupt int8_task_switch(void)
{
  (*old_int8) ();		/* chama a rotina int8 original*/
  if(single_task)		/* Se uma unica tarefa esta ativa, entao retorne */
    return;			/* sem uma escolha de tarefa */
  tasks[tswitch].ss = _SS;	/* salva o status atual da tarefa/processo na pilha */
  tasks[tswitch].sp = _SP;

  /* Se uma tarefa ou processo estava rodando quando interrompida,
     entao muda o seu status para READY*/
  if(tasks[tswitch].status == RUNNING)
    tasks[tswitch].status = READY;

  /* Verifica se algum sleeper precisa acordar */
  check_sleepers();

  /* Verifica se todos os processos estao mortos. se sim, para */
  if(all_dead())
    tasking = 0;

  /* Parou */
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

  /* Encontra novo processo */
  tswitch++;
  if(tswitch == NUM_TASKS)
    tswitch = 0;
  while(tasks[tswitch].status != READY)
  {
    tswitch++;
    if(tswitch == NUM_TASKS)
      tswitch = 0;
  }
   _SS = tasks[tswitch].ss;		/* troca o processo para o novo */
   _SP = tasks[tswitch].sp;
  tasks[tswitch].status = RUNNING;     	/* status muda para RUNNING*/
}


/*-------------------------------------------------------------------------*/
/*   task_switch()                                                         */
/*  Alternador de tarefas alternativo que o programa pode chamar para forçar
 a mudança de processo. Não diminui o contador de sleeper, porque nao ocorreu um clock
                                                                    */
/*-------------------------------------------------------------------------*/
void interrupt task_switch(void)
{
  if(single_task)
    return;

  disable();
    tasks[tswitch].ss = _SS;	/* salva o status atual do processo na pilha */
    tasks[tswitch].sp = _SP;

    /* Se um processo estava rodando quando interrompido, muda
     o seu status pra READY*/
    if(tasks[tswitch].status == RUNNING)
      tasks[tswitch].status = READY;

    /* Verfica se todos os processos estao mortos. Se assim for, para */
    if(all_dead())
      tasking = 0;

    /* Parou */
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

    /*encontra um novo processo */
    tswitch++;
    if(tswitch == NUM_TASKS)
      tswitch = 0;
    while(tasks[tswitch].status != READY)
    {
      tswitch++;
      if(tswitch == NUM_TASKS)
	tswitch = 0;
    }
    _SS = tasks[tswitch].ss;		/* Muda para o novo processo */
    _SP = tasks[tswitch].sp;
    tasks[tswitch].status = RUNNING;   	/* status muda pra RUNNING */
  enable();
}


/*-------------------------------------------------------------------------*/
/*   multitask()                                                           */
/*                                                                         */
/*   Inicia o kernel multitarefas                                     */
/*-------------------------------------------------------------------------*/
void interrupt multitask(void)
{
  disable();

    /* alterna o timer do escalonador */
    old_int8 = getvect(8);
    setvect(8, int8_task_switch);

    /*
    salva o SP (stack pointer) do programa para que, quando terminar,
    a execução continue de onde parou
    */
    oldss = _SS;
    oldsp = _SP;

    /*  seta a pilha para a primeira tarefa */
    _SS = tasks[tswitch].ss;
    _SP = tasks[tswitch].sp;
  enable();
}


/*-------------------------------------------------------------------------*/
/*   make_task()                                                           */
/*                                                                         */
/*   Retorna false se uma tarefa nao pode ser adicionada na fila.          */
/* De outra maneira, retorna true.                                         */
/*-------------------------------------------------------------------------*/
int make_task(taskptr task,
	      unsigned stck,
	      unsigned id)
{
  struct int_regs *r;

  if((id>=NUM_TASKS) || (id<0))
    return 0;

  disable();
    /* Aloca espaço para a tarefa*/
    tasks[id].stck = malloc(stck + sizeof(struct int_regs));
    r = (struct int_regs *) tasks[id].stck + stck -  sizeof(struct int_regs);

    /* inicializa a tareva */
    tasks[id].sp = FP_OFF((struct int_regs far *) r);
    tasks[id].ss = FP_SEG((struct int_regs far *) r);

    /* seta os registradores CS e IP da nova tarefa */
    r->cs = FP_SEG(task);
    r->ip = FP_OFF(task);

    /* seta os registratores  DS e ES */
    r->ds = _DS;
    r->es = _DS;

    /* liga as interrupções */
    r->flags = 0x200;

    tasks[id].status = READY;
  enable();
  return 1;
}


/*-------------------------------------------------------------------------*/
/*   free_all()                                                            */
/*                                                                         */
/*   Libera todo espaço da pilha. Esta função nao deve ser chamada         */
/* pelo seu programa                                                       */
/*                                                                         */
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
/*  Mata um processo/tarefa, o que muda seu status para DEAD              */
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
/*   Inicializa as estruturas de controle das tarefas                     */
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
/*   Para tarefas                                                          */
/*-------------------------------------------------------------------------*/
void stop_tasking(void)
{
  tasking = 0;
  task_switch();
}

/*-------------------------------------------------------------------------*/
/*   mono_task()                                                           */
/*                                                                         */
/*  Executa apenas uma tarefa                                              */
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
/*   Retorna todas as tarefas em multitask (Usado para retornar uma tarefa */
/*   depois de chamar mono_task().                                         */
/*-------------------------------------------------------------------------*/
void resume_tasking(void)
{
  single_task = 0;
}


/*-------------------------------------------------------------------------*/
/*   all_dead(void)                                                        */
/*                                                                         */
/*  retorna 1 se nenhuma tarefa possui status READ para iniciar.           */
/*  0, se pelo menos uma tarefa esta pronta para iniciar                   */
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
/*   Decrementa o sleep count                                              */
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


/*-----------------------------------------------------------------------------*/
/*   msleep()                                                                  */
/*                                                                             */
/*   Para a execução de uma tarefa por um tempo determinado de ciclos de clock */
/*-----------------------------------------------------------------------------*/
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
/*   Suspende uma tarefa enquanto nao retomada por outra                   */
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
/*   Reinicia uma tarefa previamente suspensa                                  */
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
/*  Espera por um semaforo                                               */
/*-------------------------------------------------------------------------*/
void set_semaphore(unsigned *sem)
{
  disable();
    while(*sem)
    {
      semblock(tswitch, sem);
      task_switch();
      disable();	/* trocar tarefas vai ligar as interrupções, entao precisam ser
      desligadas novamente*/
    }
    *sem = 1;
  enable();
}


/*-------------------------------------------------------------------------*/
/*   clear_semaphore                                                       */
/*                                                                         */
/*    Libera um semaforo                                                   */
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
/*   Seta uma tarefa para o status BLOCKED. Essa é uma função interna, não */
/* chamada pelo seu programa.                                              */
/*                                                                         */
/*-------------------------------------------------------------------------*/
void semblock(int id, unsigned *sem)
{
  tasks[id].status  = BLOCKED;
  tasks[id].pending = sem;
}


/*-------------------------------------------------------------------------*/
/*   restart()                                                             */
/*                                                                         */
/*   Reinicia uma tarefa que estava esperando por um semaforo especifico.  */
/* Essa é uma função interna não chamada pelo seu programa                 */
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
/*   Mostra os status de todas as tarefas. Essa função NAO deve ser chamada */
/*enquanto esta no multitarefas.                                            */
/*-------------------------------------------------------------------------*/
void task_status(void)
{
  register int i;

  if(tasking)	/* Não pode ser usado em multitarefas */
    return;

  printf("\n");

  for(i=0; i<NUM_TASKS; i++)
  {
    printf("Task %d: ",i);

    switch(tasks[i].status)
    {
      case READY:
	printf("READY\n"); /*Mostra READY */
	break;
      case RUNNING:
	printf("RUNNING\n"); /* Mostra RUNNING*/
	break;
      case BLOCKED:
	printf("BLOCKED\n"); /*mostra  BLOCKED*/
	break;
      case SUSPENDED:
	printf("SUSPENDED\n"); /* mostra SUSPENDED*/
	break;
      case SLEEPING:
	printf("SLEEPING\n"); /*mostra SLEEPING*/
	break;
      case DEAD:
	printf("DEAD\n"); /*mostra DEAD*/
	break;
    }
  }
}


/*-------------------------------------------------------------------------*/
/*                                                                         */
/* Funções de entrada e saída serializadas e reentrant (???) para multitarefas*/
/*                                                                         */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* void mputs(char *s)                                                     */
/*                                                                         */
/*   versão serializada de puts()                                          */
/*-------------------------------------------------------------------------*/
void mputs(char *s)
{
  set_semaphore(&io_out);
  puts(s);
  clear_semaphore(&io_out);
}


/*-------------------------------------------------------------------------*/
/* 	Saida de um numero                                                   */
/*-------------------------------------------------------------------------*/
void mputnum(int num)
{
  set_semaphore(&io_out);
  printf("%d", num);
  clear_semaphore(&io_out);
}


/*-------------------------------------------------------------------------*/
/* 	Versao serializada de  getche()                                     */
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
/*   mostra uma string especifica X, Y coordenadas. Essa função é reentrada       */
/* e pode ser chamada por qualquer tarefa em qualquer hora                    */
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

  v += (y*160) + x*2;	/* calcula o char loc */
  *v++ = ch;		/* escreve o caracter */
  *v = 7;		/* caracter normal */
}

/*-------------------------------------------------------------------------*/
/*	inicializa os sub-sistemas de video                                 */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* void video_mode()                                                       */
/*                                                                         */
/*   retorna o modo de video atual                                         */
/*-------------------------------------------------------------------------*/
video_mode(void)
{
  union REGS r;

  r.h.ah = 15;	/* get modo de video */
  return int86( 0x10, &r, &r) & 255;
}

/*-------------------------------------------------------------------------*/
/* void set_vid_mem()                                                      */
/*                                                                         */
/* Seta o ponteiro vid_mem para o inicio da memoria de video               */
/*-------------------------------------------------------------------------*/
void set_vid_mem(void)
{
  int vmode;

  vmode= video_mode();
  if((vmode!=2) && (vmode!=3) && (vmode!=7))
  {
    printf("/n Video must be in 80 column text mode"); /*Video deve estar no modo de texto de 80 colunas*/
    exit(1);
  }

  /* define o enderaçamento do video RAM */
  if(vmode==7)
    vid_mem = (char far *) MK_FP(0xB000, 0);
  else
    vid_mem = (char far *) MK_FP(0xB800, 0);
}
