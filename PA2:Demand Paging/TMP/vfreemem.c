/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	//kprintf("To be implemented!\n");
	STATWORD ps; 
	disable(ps);	
	struct	mblock	*p, *q;
	unsigned top;
	size = (unsigned)roundmb(size);
	if (size==0 || (((unsigned)block + size) > proctab[currpid].vmemlist->mnext) || (((unsigned)block)<((unsigned)&end)))
	{
	  restore(ps);
	  return(SYSERR);
	}
	else if((((unsigned)block+size) == (proctab[currpid].vmemlist)->mnext))
	{
	  proctab[currpid].vmemlist->mlen += size;
	  proctab[currpid].vmemlist->mnext = (unsigned)block;
	}
	else
	{
	  proctab[currpid].vmemlist->mlen += size;
	  proctab[currpid].vmemlist->mnext = proctab[currpid].vmemlist->mnext - size; 
	}
	restore(ps);
	return(OK);
}
