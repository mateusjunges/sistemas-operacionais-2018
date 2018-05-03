/* This is the master header for mm. It includes some other files
 * and defines the principal constants.
 */
 #define _POSIX_SOURCE 1 /* tell headers to include POSIX stuff */
 #define _MINIX 1 /* tell headers to include MINIX stuff */
 #define _SYSTEM 1 /* tell headers that this is the kernel */

 /* The following are so basic, all the *.c files get them automatically. */
 #include <minix/config.h> /* MUST be first */
 #include <ansi.h> /* MUST be second */
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
 
 
 
 /* This file contains the main program of the memory manager and some related
 * procedures. When MINIX starts up, the kernel runs for a little while,
 * initializing itself and its tasks, and then it runs MM and FS. Both MM
 * and FS initialize themselves as far as they can. FS then makes a call to
 * MM, because MM has to wait for FS to acquire a RAM disk. MM asks the
 * kernel for all free memory and starts serving requests.
 *
 * The entry points into this file are:
 * main: starts MM running
 * reply: reply to a process making an MM system call
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
 /* Main routine of the memory manager. */

 int error;

 mm_init(); /* initialize memory manager tables */

 /* This is MM's main loop- get work and do it, forever and forever. */
 while (TRUE) {
 /* Wait for message. */
 get_work(); /* wait for an MM system call */
 mp = &mproc[who];

 /* Set some flags. */
 error = OK;
 dont_reply = FALSE;
 err_code = -999;

 /* If the call number is valid, perform the call. */
 if (mm_call < 0 || mm_call >= NCALLS)
 error = EBADCALL;
 else
 error = (*call_vec[mm_call])();

 /* Send the results back to the user to indicate completion. */
 if (dont_reply) continue; /* no reply for EXIT and WAIT */
 if (mm_call == EXEC && error == OK) continue;
 reply(who, error, result2, res_ptr);
 }
}

/*===========================================================================*
 * get_work *
 *===========================================================================*/
 PRIVATE void get_work()
 {
 /* Wait for the next message and extract useful information from it. */

 if (receive(ANY, &mm_in) != OK) panic("MM receive error", NO_NUM);
 who = mm_in.m_source; /* who sent the message */
 mm_call = mm_in.m_type; /* system call number */
 }


 /*===========================================================================*
 * reply *
 *===========================================================================*/
 PUBLIC void reply(proc_nr, result, res2, respt)
 int proc_nr; /* process to reply to */
 int result; /* result of the call (usually OK or error #)*/
 int res2; /* secondary result */
 char *respt; /* result if pointer */
 {
 /* Send a reply to a user process. */

 register struct mproc *proc_ptr;

 proc_ptr = &mproc[proc_nr];
 /*
 * To make MM robust, check to see if destination is still alive. This
 * validy check must be skipped if the caller is a task.
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
 /* Initialize the memory manager. */

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

 /* Build the set of signals which cause core dumps. Do it the Posix
 * way, so no knowledge of bit positions is needed.
 */
 sigemptyset(&core_sset);
 for (sig_ptr = core_sigs; *sig_ptr != 0; sig_ptr++)
 sigaddset(&core_sset, *sig_ptr);

 /* Get the memory map of the kernel to see how much memory it uses,
 * including the gap between address 0 and the start of the kernel.
 */
 sys_getmap(SYSTASK, kernel_map);
 minix_clicks = kernel_map[S].mem_phys + kernel_map[S].mem_len;

 /* Initialize MM's tables. */
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

 /* Wait for FS to send a message telling the RAM disk size then go "on-line".
 */
 if (receive(FS_PROC_NR, &mess) != OK)
 panic("MM can't obtain RAM disk size from FS", NO_NUM);

 ram_clicks = mess.m1_i1;

 /* Initialize tables to all physical mem. */
 mem_init(&total_clicks, &free_clicks);

 /* Print memory information. */
 printf("\nMemory size =%5dK ", click_to_round_k(total_clicks));
 printf("MINIX =%4dK ", click_to_round_k(minix_clicks));
 printf("RAM disk =%5dK ", click_to_round_k(ram_clicks));
 printf("Available =%5dK\n\n", click_to_round_k(free_clicks));

 /* Tell FS to continue. */
 if (send(FS_PROC_NR, &mess) != OK)
 panic("MM can't sync up with FS", NO_NUM);

 /* Tell the memory task where my process table is for the sake of ps(1). */
 if ((mem = open("/dev/mem", O_RDWR)) != -1) {
 ioctl(mem, MIOCSPSINFO, (void *) mproc);
 close(mem);

 }
 }

 /* This file is concerned with allocating and freeing arbitrary-size blocks of
 * physical memory on behalf of the FORK and EXEC system calls. The key data
 * structure used is the hole table, which maintains a list of holes in memory.
* It is kept sorted in order of increasing memory address. The addresses
 * it contains refer to physical memory, starting at absolute address 0
 * (i.e., they are not relative to the start of MM). During system
 * initialization, that part of memory containing the interrupt vectors,
 * kernel, and MM are "allocated" to mark them as not available and to
 * remove them from the hole list.
 *
 * The entry points into this file are:
 * alloc_mem: allocate a given sized chunk of memory
 * free_mem: release a previously allocated chunk of memory
 * mem_init: initialize the tables when MM start up
 * max_hole: returns the largest hole currently available
 */

 #include "mm.h"
 #include <minix/com.h>

 #define NR_HOLES 128 /* max # entries in hole table */
 #define NIL_HOLE (struct hole *) 0

 PRIVATE struct hole {
 phys_clicks h_base; /* where does the hole begin? */
 phys_clicks h_len; /* how big is the hole? */
 struct hole *h_next; /* pointer to next entry on the list */
 } hole[NR_HOLES];


 PRIVATE struct hole *hole_head; /* pointer to first hole */
 PRIVATE struct hole *free_slots; /* ptr to list of unused table slots */

 FORWARD _PROTOTYPE( void del_slot, (struct hole *prev_ptr, struct hole *hp) );
 FORWARD _PROTOTYPE( void merge, (struct hole *hp) );


 /*===========================================================================*
 * alloc_mem *
 *===========================================================================*/
 PUBLIC phys_clicks alloc_mem(clicks)
 phys_clicks clicks; /* amount of memory requested */
{
 /* Allocate a block of memory from the free list using first fit. The block
 * consists of a sequence of contiguous bytes, whose length in clicks is
 * given by 'clicks'. A pointer to the block is returned. The block is
 * always on a click boundary. This procedure is called when memory is
 * needed for FORK or EXEC.
 */

 register struct hole *hp, *prev_ptr;
 phys_clicks old_base;

 hp = hole_head;
 while (hp != NIL_HOLE) {
 if (hp->h_len >= clicks) {
 /* We found a hole that is big enough. Use it. */
 old_base = hp->h_base; /* remember where it started */
 hp->h_base += clicks; /* bite a piece off */
 hp->h_len -= clicks; /* ditto */

 /* If hole is only partly used, reduce size and return. */
 if (hp->h_len != 0) return(old_base);

 /* The entire hole has been used up. Manipulate free list. */
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
 phys_clicks base; /* base address of block to free */
 phys_clicks clicks; /* number of clicks to free */
 {
 /* Return a block of free memory to the hole list. The parameters tell where
 * the block starts in physical memory and how big it is. The block is added
 * to the hole list. If it is contiguous with an existing hole on either end,
 * it is merged with the hole or holes.
 */

 register struct hole *hp, *new_ptr, *prev_ptr;

 if (clicks == 0) return;
 if ( (new_ptr = free_slots) == NIL_HOLE) panic("Hole table full", NO_NUM);
 new_ptr->h_base = base;
 new_ptr->h_len = clicks;
 free_slots = new_ptr->h_next;
 hp = hole_head;

 /* If this block's address is numerically less than the lowest hole currently
 * available, or if no holes are currently available, put this hole on the
 * front of the hole list.
 */
 if (hp == NIL_HOLE || base <= hp->h_base) {
 /* Block to be freed goes on front of the hole list. */
 new_ptr->h_next = hp;
 hole_head = new_ptr;
 merge(new_ptr);
 return;
 }

 /* Block to be returned does not go on front of hole list. */
 while (hp != NIL_HOLE && base > hp->h_base) {
 prev_ptr = hp;
 hp = hp->h_next;
 }

 /* We found where it goes. Insert block after 'prev_ptr'. */
 new_ptr->h_next = prev_ptr->h_next;
 prev_ptr->h_next = new_ptr;
 merge(prev_ptr); /* sequence is 'prev_ptr', 'new_ptr', 'hp' */
 }


 /*===========================================================================*
 * del_slot *
 *===========================================================================*/
 PRIVATE void del_slot(prev_ptr, hp)
 register struct hole *prev_ptr; /* pointer to hole entry just ahead of 'hp' */
 register struct hole *hp; /* pointer to hole entry to be removed */
 {
 /* Remove an entry from the hole list. This procedure is called when a
 * request to allocate memory removes a hole in its entirety, thus reducing
 * the numbers of holes in memory, and requiring the elimination of one
 * entry in the hole list.
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
 register struct hole *hp; /* ptr to hole to merge with its successors */
 {
 /* Check for contiguous holes and merge any found. Contiguous holes can occur
 * when a block of memory is freed, and it happens to abut another hole on
 * either or both ends. The pointer 'hp' points to the first of a series of
 * three holes that can potentially all be merged together.
 */

 register struct hole *next_ptr;

 /* If 'hp' points to the last hole, no merging is possible. If it does not,
 * try to absorb its successor into it and free the successor's table entry.
 */
 if ( (next_ptr = hp->h_next) == NIL_HOLE) return;
 if (hp->h_base + hp->h_len == next_ptr->h_base) {
 hp->h_len += next_ptr->h_len; /* first one gets second one's mem */
 del_slot(hp, next_ptr);
 } else {
 hp = next_ptr;
 }

 /* If 'hp' now points to the last hole, return; otherwise, try to absorb its
 * successor into it.
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
 /* Scan the hole list and return the largest hole. */

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
 phys_clicks *total, *free; /* memory size summaries */
 {
 /* Initialize hole lists. There are two lists: 'hole_head' points to a linked
 * list of all the holes (unused memory) in the system; 'free_slots' points to
 * a linked list of table entries that are not in use. Initially, the former
 * list has one entry for each chunk of physical memory, and the second
 * list links together the remaining table slots. As memory becomes more
 * fragmented in the course of time (i.e., the initial big holes break up into
 * smaller holes), new table slots are needed to represent them. These slots
 * are taken from the list headed by 'free_slots'.
 */

 register struct hole *hp;
 phys_clicks base; /* base address of chunk */
 phys_clicks size; /* size of chunk */
 message mess;

 /* Put all holes on the free list. */
 for (hp = &hole[0]; hp < &hole[NR_HOLES]; hp++) hp->h_next = hp + 1;
 hole[NR_HOLES-1].h_next = NIL_HOLE;
 hole_head = NIL_HOLE;
 free_slots = &hole[0];

 /* Ask the kernel for chunks of physical memory and allocate a hole for
 * each of them. The SYS_MEM call responds with the base and size of the
 * next chunk and the total amount of memory.
 */
 *free = 0;
 for (;;) {
 mess.m_type = SYS_MEM;
 if (sendrec(SYSTASK, &mess) != OK) panic("bad SYS_MEM?", NO_NUM);
 base = mess.m1_i1;
 size = mess.m1_i2;
 if (size == 0) break; /* no more? */

 free_mem(base, size);
 *total = mess.m1_i3;
 *free += size;
 }
 }
