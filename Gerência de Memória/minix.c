                                    /* Cabeçalho mestre. Inclui alguns outros arquivos e
                                     * define as constantes principais.
                                     */
 #define _POSIX_SOURCE 1             /* headers incluem POSIX */
 #define _MINIX 1                    /* cabeçalhos incluem MINIX*/
 #define _SYSTEM 1                   /* cabeçalhos incluem que este é o kernel */

                                    /* Os seguintes são básicos, todos os *.c tem automaticamente */
 #include <minix/config.h>          /* Deve ser a primeira */
 #include <ansi.h>                  /* Deve ser a segunda */
 #include <sys/types.h>
 #include <minix/const.h>
 #include <minix/type.h>

 #include <fcntl.h>
 #include <unistd.h>
 #include <minix/syslib.h>

 #include <limits.h>
 #include <errno.h>

 #include "const.h"
 #include "type.h"
 #include "proto.h"
 #include "glo.h"



 /* Esse arquivo contém o programa principal de gerência de memória e alguns procedimentos
 * relacionados. Quando o MINIX inicia, o kernel roda por alguns instantes,
 * inicializando a si mesmo e seus processos, e então roda o  MM e o  FS. Ambos MM
 * e FS inicializam na maneira do possível. FS então chama o
 * MM, porque MM tem que esperar por FS para adquirir um disco RAM. MM pede
 * ao kernel por toda a memória livre e inicia requisições ao servidor.
 *
 * Os pontos de entrada para este arquivo são:
 * main: inicia MM
 * reply: responde a um processo fazendo um chamada de sistema ao MM
 */

 #include "mm.h"
 #include <minix/callnr.h>
 #include <minix/com.h>
 #include <signal.h>
 #include <fcntl.h>
 #include <sys/ioctl.h>
 #include "mproc.h"
 #include "param.h"

 FORWARD _PROTOTYPE( void get_work, (void) );
 FORWARD _PROTOTYPE( void mm_init, (void) );

 /*===========================================================================*
 * main *
 *===========================================================================*/
 PUBLIC void main()
 {
 /* Rotina principal da gerência de memória. */

 int error;

 mm_init(); /* inicializa as tabelas de gerência de memória */

 /*  Loop principal do MM, que roda para sempre  */
 while (TRUE) {
 /* espera mensagem. */
 get_work(); /* espera uma chamada de sistema por gerência de memória */
 mp = &mproc[who];

 /* Seta algumas flags. */
 error = OK;
 dont_reply = FALSE;
 err_code = -999;

 /* Se o número de chamada é válido, realiza a tarefa. */
 if (mm_call < 0 || mm_call >= NCALLS)
 error = EBADCALL;
 else
 error = (*call_vec[mm_call])();

 /* Envia os resultados de volta para o usuário, indicando que completou */
 if (dont_reply) continue; /* Sem resposta para EXIT e WAIT */
 if (mm_call == EXEC && error == OK) continue;
 reply(who, error, result2, res_ptr);
 }
}

/*===========================================================================*
 * função get_work *
 *===========================================================================*/
 PRIVATE void get_work()
 {
                            /* Espera uma próxima chamada e extrai alguma informação útil dela */

 if (receive(ANY, &mm_in) != OK) panic("MM receive error", NO_NUM);
 who = mm_in.m_source;      /* quem enviou a mensagem */
 mm_call = mm_in.m_type;    /* número da chamada de sistema */
 }


 /*===========================================================================*
 * reply *
 *===========================================================================*/
 PUBLIC void reply(proc_nr, result, res2, respt)
 int proc_nr;       /* processo ao qual responder */
 int result;        /* resultado da chamada (usualmente OK ou erro #)*/
 int res2;          /* segundo resultado */
 char *respt;       /* resultado do ponteiro */
 {
                    /* Envia uma resposta a um processo de um usuário. */

 register struct mproc *proc_ptr;

 proc_ptr = &mproc[proc_nr];
             /*
             * Essa verificação de validade deve ser ignorada se o chamador for uma tarefa.
             * Para tornar o MM robusto, verifica se o destino ainda está vivo
             * Essa verificação checa se o chamador é uma tarefa.
             */
 if ((who >=0) && ((proc_ptr->mp_flags&IN_USE) == 0 ||
 (proc_ptr->mp_flags&HANGING))) return;

 reply_type = result;
 reply_i1 = res2;
 reply_p1 = respt;
 if (send(proc_nr, &mm_out) != OK) panic("MM can't reply", NO_NUM);
 }


 /*===========================================================================*
 * mm_init *
 *===========================================================================*/
 PRIVATE void mm_init()
 {
 /* Inicializa o gerênciador de memória */

 static char core_sigs[] = {
 SIGQUIT, SIGILL, SIGTRAP, SIGABRT,
 SIGEMT, SIGFPE, SIGUSR1, SIGSEGV,
 SIGUSR2, 0 };
 register int proc_nr;
 register struct mproc *rmp;
 register char *sig_ptr;
 phys_clicks ram_clicks, total_clicks, minix_clicks, free_clicks, dummy;
 message mess;
 struct mem_map kernel_map[NR_SEGS];
 int mem;

            /*
            * Constrói um conjunto de sinais que causam duplicações de núcleo.
            * Faz isso da maneira do POSIX, então nenhum conhecimento de posições
            * bit é necessário
            */

 sigemptyset(&core_sset);
 for (sig_ptr = core_sigs; *sig_ptr != 0; sig_ptr++)
 sigaddset(&core_sset, *sig_ptr);

             /*
             * Obtém o mapa de memória do kernel para ver quanta memória isso usa,
             * incluindo o gap entre o endereço 0 e o início do kernel
             */
 sys_getmap(SYSTASK, kernel_map);
 minix_clicks = kernel_map[S].mem_phys + kernel_map[S].mem_len;

            /* inicializa as tabelas de gerênciamento de memória */

 for (proc_nr = 0; proc_nr <= INIT_PROC_NR; proc_nr++) {
 rmp = &mproc[proc_nr];
 rmp->mp_flags |= IN_USE;
 sys_getmap(proc_nr, rmp->mp_seg);
 if (rmp->mp_seg[T].mem_len != 0) rmp->mp_flags |= SEPARATE;
 minix_clicks += (rmp->mp_seg[S].mem_phys + rmp->mp_seg[S].mem_len)
 - rmp->mp_seg[T].mem_phys;
}
 mproc[INIT_PROC_NR].mp_pid = INIT_PID;
 sigemptyset(&mproc[INIT_PROC_NR].mp_ignore);
 sigemptyset(&mproc[INIT_PROC_NR].mp_catch);
 procs_in_use = LOW_USER + 1;

             /*
             * Espera por FS para enviar uma mensagem dizendo ao disco RAM o tamanho que vai on-line
             */
 if (receive(FS_PROC_NR, &mess) != OK)
 panic("MM can't obtain RAM disk size from FS", NO_NUM);

 ram_clicks = mess.m1_i1;

            /* Inicializa as tabelas para toda a memória física. */

 mem_init(&total_clicks, &free_clicks);

        /* Mostra as informações de memória */

 printf("\nMemory size =%5dK ", click_to_round_k(total_clicks));
 printf("MINIX =%4dK ", click_to_round_k(minix_clicks));
 printf("RAM disk =%5dK ", click_to_round_k(ram_clicks));
 printf("Available =%5dK\n\n", click_to_round_k(free_clicks));

        /* Diz ao FS para continuar. */

 if (send(FS_PROC_NR, &mess) != OK)
 panic("MM can't sync up with FS", NO_NUM);


        /* Diz ao processo de memória onde minha tabela de processo está para o ps(1) */

 if ((mem = open("/dev/mem", O_RDWR)) != -1) {
 ioctl(mem, MIOCSPSINFO, (void *) mproc);
 close(mem);

 }
 }

             /*
             * Este arquivo está preocupado com a alocação de memória e liberação
             * de blocos de tamanho arbitrário de blocos de memória em nome das chamadas
             * de sistema FORK e EXEC. Os dados principais da estrutura usada é a tabela
             * de furos, que mantém uma lista de furos na memórria. Ele também é mantido
             * classificado em ordem crescente de endereço de memória. Os endereços contido
             * refere-se a memória física, começando no endereço 0.
             * (ou seja, eles não são relacionados ao início da gerencia de memória)
             * Durante a inicialização do sistema, a parte de memória que contém os
             * vetores de interrupção, kernel e gerência de memória são alocados para marcá-los
             * como disponíveis e removidos da lista de buracos
             * Os pontos de entrada para este arquivo são:
             * The entry points into this file are:
             * alloc_mem: aloca um dado pedaço de memória
             * free_mem: libera um pedaço previamente alocado de memória
             * mem_init: inicializa as tabelas quando a gerência de memória inicia
             * max_hole: retorna o maior burado atualmente disponível
             */

 #include "mm.h"
 #include <minix/com.h>

 #define NR_HOLES 128       /* max # de entradas na tabela de buracos */
 #define NIL_HOLE (struct hole *) 0

 PRIVATE struct hole {
 phys_clicks h_base;                        /* onde inicia o buraco? */
 phys_clicks h_len;                         /* qual o tamanho do buraco? */
 struct hole *h_next;                       /* ponteiro da próxima entrada na lista */
 } hole[NR_HOLES];


 PRIVATE struct hole *hole_head;            /* ponteiro para o primeiro buraco */
 PRIVATE struct hole *free_slots;           /* ponteiro apra a lista de buracos não utilizadas na tabela */

 FORWARD _PROTOTYPE( void del_slot, (struct hole *prev_ptr, struct hole *hp) );
 FORWARD _PROTOTYPE( void merge, (struct hole *hp) );


 /*===========================================================================*
 * alloc_mem *
 *===========================================================================*/
 PUBLIC phys_clicks alloc_mem(clicks)
 phys_clicks clicks; /* amount of memory requested */
{
             /*
             * Aloca um bloco de memória da lista livre usando first fit.
             * O bloco consite na sequência de bytes contíguos, cujo comprimento
             * em cliques é dado por cliques. Um ponteiro para o bloco é retornado.
             * O bloco está sempre em um limite de cliques.
             * Este procedimento é chamado quando memória é necessária pelo FORK ou EXEC
             */

 register struct hole *hp, *prev_ptr;
 phys_clicks old_base;

 hp = hole_head;
 while (hp != NIL_HOLE) {
 if (hp->h_len >= clicks) {
                            /* Encontrado um buraco grande o suficiente. Use-o. */
 old_base = hp->h_base;     /* Relembre onde inicia */
 hp->h_base += clicks;      /* pega um pedaço */
 hp->h_len -= clicks;       /* idem */

                            /* Se um buraco está parcialmente usado, reduz o seu tamanho e retorna. */
 if (hp->h_len != 0) return(old_base);

                            /* O buraco inteiro foi usado. Manipula a lista livre. */
 del_slot(prev_ptr, hp);
 return(old_base);
 }

 prev_ptr = hp;
 hp = hp->h_next;
 }
 return(NO_MEM);
 }


 /*===========================================================================*
 * free_mem *
 *===========================================================================*/
 PUBLIC void free_mem(base, clicks)
 phys_clicks base;              /* endereço base do bloco a ser liberado */
 phys_clicks clicks;            /* número de cliques para liberar */
 {
             /*
             * Retorna um bloco de memória livre para lista de livres. Os parâmetros dizem
             * onde o bloco inicia na memória física e qual o tamanho dele. O bloco é
             * adicionado a lista de buracos. Se ele é contíguo com um buraco existente
             * em ambas as extremidades, ele é fundido com o buraco ou buracos
             */

 register struct hole *hp, *new_ptr, *prev_ptr;

 if (clicks == 0) return;
 if ( (new_ptr = free_slots) == NIL_HOLE) panic("Hole table full", NO_NUM);
 new_ptr->h_base = base;
 new_ptr->h_len = clicks;
 free_slots = new_ptr->h_next;
 hp = hole_head;

             /*
             * Se este endereço de bloco é numericamente menor que o menor buraco atualmente
             * disponível, ou se nenhum buraco está atualmente disponível, coloca este buraco
             * na frente da lista de buracos.
             */
 if (hp == NIL_HOLE || base <= hp->h_base) {

            /*  Bloco a ser liberado na frente da lista de buracos  */

 new_ptr->h_next = hp;
 hole_head = new_ptr;
 merge(new_ptr);
 return;
 }

            /* Bloco a ser retornado não vai a frente da lista de buracos. */

 while (hp != NIL_HOLE && base > hp->h_base) {
 prev_ptr = hp;
 hp = hp->h_next;
 }

 /* Onde vai. Insere o bloco depois de 'prev_ptr'. */

 new_ptr->h_next = prev_ptr->h_next;
 prev_ptr->h_next = new_ptr;
 merge(prev_ptr);   /* A sequência é 'prev_ptr', 'new_ptr', 'hp' */
 }


 /*===========================================================================*
 * del_slot *
 *===========================================================================*/
 PRIVATE void del_slot(prev_ptr, hp)
 register struct hole *prev_ptr;        /* pointer to hole entry just ahead of 'hp' */
 register struct hole *hp;              /* pointer to hole entry to be removed */
 {
                             /*
                             * Remove uma entrada da lista de buracos. Este procedimento é chamado quando um
                             * chamada para alocar memória remove um buraco na sua totalidade, reduzindo
                             * assim, o número de buracos na memória e a eliminação de uma entrada na lista de buracos.
                             */

 if (hp == hole_head)
 hole_head = hp->h_next;
 else
 prev_ptr->h_next = hp->h_next;

 hp->h_next = free_slots;
 free_slots = hp;
 }


 /*===========================================================================*
 * merge *
 *===========================================================================*/
 PRIVATE void merge(hp)
 register struct hole *hp; /* ponteiro para o buraco para juntar com seus sucessores */
 {

             /*
             * Verifica se há buracos e junta os encontrados. Buracos contíguos
             * podem ocorer quando um bloco de memória está livre, e isso acontece
             * para encostar em outro buraco uma ou ambas as extremidades.
             * O ponteiro 'hp' aponta para o primeiro de uma série de buraco que
             * potencialmente podem estar todos juntos
             */

 register struct hole *next_ptr;

             /* Se 'hp' aponta para o último buraco, não é possível juntá-lo.
             * Se não, tenta absorver seu sucessor e libera a entrada da tabela do sucessor.
             */

 if ( (next_ptr = hp->h_next) == NIL_HOLE) return;
 if (hp->h_base + hp->h_len == next_ptr->h_base) {
 hp->h_len += next_ptr->h_len; /* primeiro recebe o segundo */
 del_slot(hp, next_ptr);
 } else {
 hp = next_ptr;
 }

             /* Se 'hp' agora aponta para o último buraco, retorna; De outra maneira,
             * tenta absorver seu sucessor.
             */
 if ( (next_ptr = hp->h_next) == NIL_HOLE) return;
 if (hp->h_base + hp->h_len == next_ptr->h_base) {
 hp->h_len += next_ptr->h_len;
 del_slot(hp, next_ptr);
 }
}


 /*===========================================================================*
 * max_hole *
 *===========================================================================*/
 PUBLIC phys_clicks max_hole()
 {
            /* Verifica a lista de buracos e retorna o maior deles. */

 register struct hole *hp;
 register phys_clicks max;

 hp = hole_head;
 max = 0;
 while (hp != NIL_HOLE) {
 if (hp->h_len > max) max = hp->h_len;
 hp = hp->h_next;
 }
 return(max);
 }


 /*===========================================================================*
 * mem_init *
 *===========================================================================*/
 PUBLIC void mem_init(total, free)
 phys_clicks *total, *free; /* resumos de tamanho de memória */
 {
             /* Inicializa a lista de buracos. São duas listas: 'hole_head' aponta
             * para uma lista encadeada de buracos (memória não usada) no sistema;
             * 'free_slots' aponta para uma lista encadeada de entradas de tabela que
             * não estão em uso.
             * Inicialmente, a antiga lista tem uma entrada para cada pedaço de memória física
             * e a segunda lista encadeada junta o restante da tabela de slots.
             * Como a memória se torna mais fragmentado no decorrer do tempo (ou seja, os grandes
             * buracos iniciais se dividem em buracos menores), novos slots de tabela são
             * necessários para representá-los. Esses slots são retirados da lista
             * liderada por 'free_slots'
             */

 register struct hole *hp;
 phys_clicks base;  /* endereço base */
 phys_clicks size;  /* tamnaho */
 message mess;

                    /* Coloca todos os buracos na lista */

 for (hp = &hole[0]; hp < &hole[NR_HOLES]; hp++) hp->h_next = hp + 1;
 hole[NR_HOLES-1].h_next = NIL_HOLE;
 hole_head = NIL_HOLE;
 free_slots = &hole[0];

                     /*
                     * Pergunte ao kernel por pedaçoes de memória física e alocar um buraco para
                     * cada um deles. A chamada SYS_MEM responde com a base e tamanho do próximo
                     * pecaço e a quantidade total de memória.
                     */
 *free = 0;
 for (;;) {
 mess.m_type = SYS_MEM;
 if (sendrec(SYSTASK, &mess) != OK) panic("bad SYS_MEM?", NO_NUM);
 base = mess.m1_i1;
 size = mess.m1_i2;
 if (size == 0) break; /* sem mais? */

 free_mem(base, size);
 *total = mess.m1_i3;
 *free += size;
 }
 }
