/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	//kprintf("To be implemented!\n");
	STATWORD ps;
	disable(ps);
	int bsmNum;
	int status = -1;
	int newpid;
	struct mblock* bsm_base_for_pid;
	status = get_bsm(&bsmNum);
	//kprintf("bsm number %d",bsmNum);
	if(status == SYSERR)
	{
		//kprintf("SYSERR 1 \n");
		restore(ps);
		return status;
	}
	//first 4 pd will be handled in create call
	newpid = create(procaddr, ssize, priority, name, nargs, args);
	kprintf("vcreate pid : %d \n",newpid);
	status = -1;
	status = bsm_map(newpid, 4096, bsmNum , hsize);
	if(status == SYSERR)
	{
		//kprintf("SYSERR 2 \n");
		restore(ps);
		return status;
	}
	bsm_tab[bsmNum].bs_private_flag = 1;
	proctab[newpid].store = bsmNum;
	proctab[newpid].vhpno = 4096;
	proctab[newpid].vhpnpages = hsize;
	proctab[newpid].flag_private_bsm = 1; // private backing store is stored in store
	proctab[newpid].vmemlist->mnext = 4096 * NBPG; // since the process should get the virtualization that it has all the memory for itself
	proctab[newpid].vmemlist->mlen = hsize * NBPG;
	bsm_base_for_pid = BACKING_STORE_BASE + (bsmNum * BACKING_STORE_UNIT_SIZE);
	bsm_base_for_pid->mlen = hsize * NBPG;
	bsm_base_for_pid->mnext = NULL;
	restore(ps);
	//kprintf("return ok \n");
	return newpid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
